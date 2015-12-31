#include "stdafx.h"
#include "ExceptionHandler.h"
#include "Helper.h"
#include <StackWalker\StackWalker.h>

// https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273/
#define CXX_EXCEPTION_MAGIC 0xE06D7363
struct CXX_EXCEPTION_DETAIL {
	DWORD _0;
	DWORD _1;
	DWORD _2;
	DWORD catchableTypesPtr;
};

struct CXX_EXCEPTION_CATCHABLE_TYPES {
	DWORD count;
	DWORD catchableTypeHead;
};

struct CXX_EXCEPTION_CATCHABLE_TYPE {
	DWORD _0;
	DWORD typeInfo;
	DWORD _2;
	DWORD _3;
	DWORD _4;
	DWORD _5;
	DWORD _6;
};

static std::string join(const std::vector<std::string>& vec, const char* delim) {
	std::stringstream res;
	copy(vec.begin(), vec.end(), std::ostream_iterator<std::string>(res, delim));
	return res.str();
}

class MyStackWalker : public StackWalker {
protected:
	std::vector<std::string>& _arr;

	virtual void OnOutput(LPCSTR szText) {
		_arr.push_back(szText);
		StackWalker::OnOutput(szText);
	}

	virtual void OnSymInit(LPCSTR szSearchPath, DWORD symOptions, LPCSTR szUserName) {}
	virtual void OnLoadModule(LPCSTR img, LPCSTR mod, DWORD64 baseAddr, DWORD size, DWORD result, LPCSTR symType, LPCSTR pdbName, ULONGLONG fileVersion) {}
	virtual void OnDbgHelpErr(LPCSTR szFuncName, DWORD gle, DWORD64 addr) {}
	virtual void OnCallstackEntry(CallstackEntryType eType, CallstackEntry &entry) {
		CHAR buffer[STACKWALK_MAX_NAMELEN];
		if ((eType != lastEntry) && (entry.offset != 0)) {
			if (entry.name[0] == 0)
				strcpy_s(entry.name, "?");
			if (entry.undName[0] != 0)
				strcpy_s(entry.name, entry.undName);
			if (entry.undFullName[0] != 0)
				strcpy_s(entry.name, entry.undFullName);
			if (entry.lineFileName[0] == 0) {
				strcpy_s(entry.lineFileName, "?");
				if (entry.moduleName[0] == 0)
					strcpy_s(entry.moduleName, "?");
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s): %s: %s\n", (LPVOID)entry.offset, entry.moduleName, entry.lineFileName, entry.name);
			} else
				_snprintf_s(buffer, STACKWALK_MAX_NAMELEN, "%p (%s): %s (%d): %s\n", (LPVOID)entry.offset, entry.moduleName, entry.lineFileName, entry.lineNumber, entry.name);
			OnOutput(buffer);
		}
	}

public:
	MyStackWalker(std::vector<std::string>& arr) : StackWalker(), _arr(arr) {}
};

static const wchar_t * GetFriendlyExceptionCode(DWORD code) {
	switch (code) {
		case CXX_EXCEPTION_MAGIC:
			return L"CxxException";
		case EXCEPTION_ACCESS_VIOLATION:
			return L"AccessViolation";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
			return L"ArrayBoundsExceeded";
		case EXCEPTION_BREAKPOINT:
			return L"Breakpoint";
		case EXCEPTION_DATATYPE_MISALIGNMENT:
			return L"DatatypeMisalignment";
		case EXCEPTION_FLT_DENORMAL_OPERAND:
			return L"FltDenormalOperand";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
			return L"FltDivideByZero";
		case EXCEPTION_FLT_INEXACT_RESULT:
			return L"FltInexactResult";
		case EXCEPTION_FLT_INVALID_OPERATION:
			return L"FltInvalidOperation";
		case EXCEPTION_FLT_OVERFLOW:
			return L"FltOverflow";
		case EXCEPTION_FLT_STACK_CHECK:
			return L"FltStackCheck";
		case EXCEPTION_FLT_UNDERFLOW:
			return L"FltUnderflow";
		case EXCEPTION_ILLEGAL_INSTRUCTION:
			return L"IllegalInstruction";
		case EXCEPTION_IN_PAGE_ERROR:
			return L"InPageError";
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
			return L"IntDivideByZero";
		case EXCEPTION_INT_OVERFLOW:
			return L"IntOverflow";
		case EXCEPTION_INVALID_DISPOSITION:
			return L"InvalidDisposition";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
			return L"NoncontinuableException";
		case EXCEPTION_PRIV_INSTRUCTION:
			return L"PrivInstruction";
		case EXCEPTION_SINGLE_STEP:
			return L"SingleStep";
		case EXCEPTION_STACK_OVERFLOW:
			return L"StackOverflow";
	}
	return 0;
}

static void GenerateErrorMessage(EXCEPTION_POINTERS* e, std::wostringstream& msg) {
	const wchar_t* code = GetFriendlyExceptionCode(e->ExceptionRecord->ExceptionCode);
	if (code) {
		msg << code;
	} else {
		msg << "0x" << std::uppercase << std::setfill(L'0') << std::setw(2 * sizeof(DWORD)) << std::hex << e->ExceptionRecord->ExceptionCode;
	}

	if (e->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && e->ExceptionRecord->NumberParameters == 2) {
		ULONG_PTR flag = e->ExceptionRecord->ExceptionInformation[0];
		ULONG_PTR ptr = e->ExceptionRecord->ExceptionInformation[1];
		msg << "(";
		switch (flag) {
			case 0: msg << "Read"; break;
			case 1: msg << "Write"; break;
			case 8: msg << "Execute"; break;
			default:
				msg << flag;
		}
		msg << "," << "0x" << std::uppercase << std::setfill(L'0') << std::setw(2 * sizeof(ULONG_PTR)) << std::hex << ptr << ")";

	} else if (e->ExceptionRecord->ExceptionCode == CXX_EXCEPTION_MAGIC && 
#ifdef __x86_64
		e->ExceptionRecord->NumberParameters == 4
#else
		e->ExceptionRecord->NumberParameters == 3
#endif
		) { // https://blogs.msdn.microsoft.com/oldnewthing/20100730-00/?p=13273/
		msg << "(";

		ULONG_PTR offset = 0;
#ifdef __x86_64
		offset = e->ExceptionRecord->ExceptionInformation[3];
#endif

		CXX_EXCEPTION_DETAIL *detail = (CXX_EXCEPTION_DETAIL*)e->ExceptionRecord->ExceptionInformation[2];
		CXX_EXCEPTION_CATCHABLE_TYPES *types = (CXX_EXCEPTION_CATCHABLE_TYPES *)(offset + detail->catchableTypesPtr);
		CXX_EXCEPTION_CATCHABLE_TYPE *type = (CXX_EXCEPTION_CATCHABLE_TYPE *)(offset + (&types->catchableTypeHead)[0]);
		std::type_info* info = (std::type_info*) (offset + type->typeInfo);
		std::string className = info->name();

		msg << "0x" << std::uppercase << std::setfill(L'0') << std::setw(2 * sizeof(ULONG_PTR)) << std::hex << (LPVOID)e->ExceptionRecord->ExceptionInformation[1];
		msg << ", " << ANSIToUtf16(className) << ")";

		if (className.compare(typeid(class std::runtime_error).name()) == 0) {
			std::runtime_error* err = (std::runtime_error*)e->ExceptionRecord->ExceptionInformation[1];
			msg << L": " << Utf8ToUtf16(err->what());
		}
	} else if (e->ExceptionRecord->NumberParameters > 0) {
		msg << "(";
		for (DWORD i = 0; i < e->ExceptionRecord->NumberParameters; i++) {
			ULONG_PTR value = e->ExceptionRecord->ExceptionInformation[i];
			if (i != 0) msg << ",";
			msg << "0x" << std::uppercase << std::setfill(L'0') << std::setw(2 * sizeof(ULONG_PTR)) << std::hex << value;
		}
		msg << ")";
	}
}

static LONG CALLBACK MyUnhandledExceptionFilter(EXCEPTION_POINTERS* e) {
	std::vector<std::string> stack;
	MyStackWalker sw(stack);
	sw.ShowCallstack(GetCurrentThread(), e->ContextRecord);

	std::wostringstream msg;
	GenerateErrorMessage(e, msg);
	msg << L"\n\n" << ANSIToUtf16(join(stack, ""));

	std::wstring message = msg.str();

	int button = IDCANCEL;
	HRESULT result = TaskDialog(NULL, NULL, L"NekoScript", L"Unhandled Native Exception", message.c_str(), TDCBF_CANCEL_BUTTON | TDCBF_RETRY_BUTTON, TD_ERROR_ICON, &button);
	if (result != S_OK) {
		button = MessageBoxW(NULL, message.c_str(), L"NekoScript: Unhandled Native Exception", MB_ICONERROR | MB_RETRYCANCEL | MB_DEFBUTTON2);
	}

	if (button == IDRETRY) {
		return EXCEPTION_CONTINUE_SEARCH;
	}
	return EXCEPTION_EXECUTE_HANDLER;
}

void InstallExceptionHandler() {
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
}