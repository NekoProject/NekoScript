#include "stdafx.h"
#include <duktape\duktape.h>
#include "NekoScript.h"
#include "Helper.h"
#include "ExceptionHandler.h"
#include "JSProcess.h"
#include "JSEngineExtra.h"
#include "JSFileSystem.h"
#include "JSEventEmitter.h"
#include "NKWinAPI.h"
#include "NKWinVersionInfo.h"
#include "NKWinTaskDialog.h"
#include <lzma\CPP\Common\CommandLineParser.h>

duk_ret_t InitDuktapeContext(duk_context *ctx) {
	duk_push_global_object(ctx);
	duk_push_string(ctx, "global");
	duk_push_global_object(ctx);
	duk_def_prop(ctx, -3, DUK_DEFPROP_HAVE_VALUE |
		DUK_DEFPROP_HAVE_WRITABLE | 0 |
		DUK_DEFPROP_HAVE_ENUMERABLE | 0 |
		DUK_DEFPROP_HAVE_CONFIGURABLE | 0 |
		DUK_DEFPROP_FORCE);
	duk_pop(ctx);
	_ASSERTE(duk_get_top(ctx) == 0);

	JSEngineExtra::setup(ctx);
	JSProcess::setup(ctx);
	JSFileSystem::setup(ctx);
	NKWinAPI::setup(ctx); duk_put_global_string(ctx, "NKWinAPI");
	NKWinVersionInfo::setup(ctx); duk_put_global_string(ctx, "NKWinVersionInfo");

#ifndef NEKO_MINIMAL
	JSEventEmitter::setup(ctx);
	NKWinTaskDialog::setup(ctx); duk_put_global_string(ctx, "NKWinTaskDialog");
#endif

	duk_push_null(ctx);
	return 1;
}

void ShowConsole() {
	AllocConsole();
	FILE* useless;
	freopen_s(&useless, "CONOUT$", "w", stdout);
	freopen_s(&useless, "CONOUT$", "w", stderr);
	freopen_s(&useless, "CONIN$", "r", stdin);
}

bool isConsoleMode = false;
bool IsConsoleMode() {
	return isConsoleMode;
}

void Fail(std::wstring title, std::wstring content) {
	if (IsConsoleMode()) {
		std::wcerr << title << ": " << content;
	} else {
		TaskDialog(NULL, NULL, L"NekoScript", title.c_str(), content.c_str(), TDCBF_CANCEL_BUTTON, TD_ERROR_ICON, NULL);
	}
}

duk_context *CreateDuktapeContext() {
	duk_context *ctx = duk_create_heap_default();
	if (duk_safe_call(ctx, InitDuktapeContext, 0, 1) != DUK_EXEC_SUCCESS) {
		Fail(L"Initialization Failure", Utf8ToUtf16(std::string(duk_safe_to_string(ctx, -1))));
		abort();
	}
	duk_pop(ctx);

	return ctx;
}

int AppMain() {
	InstallExceptionHandler();

	int retCode = 0;
	std::wstring fileName;

	SetProcessDPIAware();

	UStringVector commandStrings;
	NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);
	_ASSERTE(commandStrings.Size() >= 1);

	int scriptArgvOffset = 1;
	if (commandStrings.Size() >= 2) {
		fileName = commandStrings[1];
		scriptArgvOffset = 2;
	}
	if (fileName.empty()) {
		fileName = L"D:\\git\\qtwrap\\installer\\windows\\dist\\app.neko.js";
	}

	JSProcess::_execPath = commandStrings[0];
	JSProcess::_scriptPath = fileName;
	for (unsigned int i = scriptArgvOffset; i < commandStrings.Size(); i++) {
		JSProcess::_scriptArgv.push_back(std::wstring(commandStrings[i]));
	}

	if (!FileExists(fileName.c_str())) {
		Fail(L"Module Not Found", fileName + L" cannot be found.");
		return -1;
	}

	duk_context *ctx = CreateDuktapeContext();
	if (fileName.size() > 4 && (fileName.substr(fileName.size() - 4).compare(L".jsc") == 0 || fileName.substr(fileName.size() - 4).compare(L".bin") == 0)) {
		duk_push_string(ctx, Utf16ToUtf8(fileName).c_str());
		if (duk_safe_call(ctx, JSFileSystem::readFileSync, 1, 1) != DUK_EXEC_SUCCESS) {
			Fail(L"Bytecode Error", Utf8ToUtf16(std::string(duk_safe_to_string(ctx, -1))));
			return -1;
		}
		if (duk_safe_call(ctx, JSEngineExtra::loadBytecode, 1, 1) != DUK_EXEC_SUCCESS) {
			Fail(L"Bytecode Error", Utf8ToUtf16(std::string(duk_safe_to_string(ctx, -1))));
			return -1;
		}
	} else {
		duk_push_string(ctx, Utf16ToUtf8(fileName).c_str());
		if (duk_pcompile_string_filename(ctx, 0, ReadWholeFileAsString(fileName).c_str()) != DUK_EXEC_SUCCESS) {
			Fail(L"Source Error", Utf8ToUtf16(std::string(duk_safe_to_string(ctx, -1))));
			return -1;
		}
	}

	duk_int_t rc = duk_pcall(ctx, 0);
	if (rc != DUK_EXEC_SUCCESS) {
		Fail(L"Uncaught Error", Utf8ToUtf16(JSGetErrorStack(ctx)));
		return -1;
	}

	duk_pop(ctx);
	duk_destroy_heap(ctx);
	
	return retCode;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	// ShowConsole();
	isConsoleMode = false;
	return AppMain();
}

int main(int argc, char** argv) {
	isConsoleMode = true;
	return AppMain();
}