#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>
#include <uv.h>
#include "NekoContext.h"

std::wstring Utf8ToUtf16(const std::string &s);
std::string Utf16ToUtf8(const std::wstring &s);
std::wstring ANSIToUtf16(const std::string &s);
std::string Utf16ToANSI(const std::wstring &s);
std::string ReadWholeFileAsString(const std::wstring &fname);
bool FileExists(LPCTSTR szPath);
std::wstring GetModulePath(HMODULE module);

std::string JSGetPropString(duk_context *ctx, const std::string& name);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index);
std::string JSGetPropString(duk_context *ctx, const std::string& name, bool* isNull);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index, bool* isNull);
bool JSGetPropBoolean(duk_context *ctx, const std::string& name);
bool JSGetPropBoolean(duk_context *ctx, duk_uarridx_t arr_index);
std::string JSReplaceString(duk_context *ctx, const std::string& str, const std::string& pattern, const std::string& replacement, const std::string& option = "g");
std::string JSGetErrorStack(duk_context *ctx);

void JSPushWin32ErrorObject(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function);
__declspec(noreturn) void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, const char * function = 0);
__declspec(noreturn) void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function = 0);

#define JSThrowWin32Error(...)  \
	JSThrowWin32ErrorRaw((const char *) (__FILE__), (duk_int_t) (__LINE__), __VA_ARGS__)

unsigned int DynamicInvokeStdcall(FARPROC proc, int argc, void* argv[]);
unsigned int DynamicInvokeCdecl(void *proc, int argc, void* argv[]);

class JSThrowScope_t {
protected:
	struct data_t {
		duk_context *ctx;
		void *proc;
		int argc;
		void **argv;
	};
	data_t data;

	JSThrowScope_t(duk_context *ctx) {
		data.ctx = ctx;
		data.proc = 0;
	}
	~JSThrowScope_t() {}

	template<typename... Args> inline void pass(Args&&...) {}
	template<typename T>inline int convert(std::vector<void *>& list, T i) {
		list.push_back((void *)i);
		return 0;
	}
	template<typename... Args> inline void expand(std::vector<void *>& list, Args&&... args) {
		pass(convert(list, args)...);
	}

public:
	template<typename... Args>
	void set(void *func, Args... args) {
		if (data.proc) {
			throw std::runtime_error("JSThrowScope_t::set called twice!");
		}

		std::vector<void *> argvList;
		expand(argvList, args...);

		int argc = argvList.size();
		data.proc = func;
		data.argc = argc;
		data.argv = 0;
		if (argc > 0) {
			void **argv = new void*[argc];
			for (int i = 0; i < argc; i++) {
				argv[i] = argvList[i];
			}
			data.argv = argv;
		}
	}

	static JSThrowScope_t *create(duk_context *_ctx) {
		return new JSThrowScope_t(_ctx);
	}

	static void invoke(JSThrowScope_t *self) {
		data_t data = self->data;
		delete self;

		if (data.proc) {
			DynamicInvokeCdecl(data.proc, data.argc, data.argv);
		}
	}
};

#define JSThrowScope JSThrowScope_t *__throw_scope = JSThrowScope_t::create(ctx); auto __throw_lambda = [&]()
#define JSThrowScopeEnd ; __throw_lambda(); JSThrowScope_t::invoke(__throw_scope);
#define JSThrowScopeBreak return;

#define JSThrowScopeWin32Error(code, func) \
	do { \
		__throw_scope->set(\
			(void *)(void(*)(const char *, duk_int_t, duk_context *, DWORD, const char *))&JSThrowWin32ErrorRaw, \
			(const char *)(__FILE__), (duk_int_t)(__LINE__), ctx, (code), (func)\
		); JSThrowScopeBreak;\
	} while(0)
#define JSThrowScopeWin32ErrorLast(func) JSThrowScopeWin32Error(GetLastError(), func)
