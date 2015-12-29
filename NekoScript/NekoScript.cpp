#include "stdafx.h"
#include <duktape\duktape.h>
#include "NekoScript.h"
#include "Helper.h"
#include "JSProcess.h"
#include "JSEngineExtra.h"
#include "JSFileSystem.h"
#include "JSEventEmitter.h"
#include "NKWinTaskDialog.h"
#include <lzma\CPP\Common\CommandLineParser.h>

duk_context *CreateDuktapeContext() {
	duk_context *ctx = duk_create_heap_default();

	JSEngineExtra::setup(ctx);
	JSProcess::setup(ctx);
	JSFileSystem::setup(ctx);

#ifndef NEKO_MINIMAL
	JSEventEmitter::setup(ctx);

	NKWinTaskDialog::setup(ctx);
	duk_put_global_string(ctx, "NKWinTaskDialog");
#endif

	return ctx;
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
		TaskDialog(NULL, NULL, L"NekoScript", title.c_str(), content.c_str(), TDCBF_CLOSE_BUTTON, TD_ERROR_ICON, NULL);
	}
}

int AppMain() {
	int retCode = 0;
	std::wstring fileName;

	UStringVector commandStrings;
	NCommandLineParser::SplitCommandLine(GetCommandLineW(), commandStrings);
	_ASSERTE(commandStrings.Size() >= 1);

	int scriptArgvOffset = 1;
	if (commandStrings.Size() >= 2) {
		fileName = commandStrings[1];
		scriptArgvOffset = 2;
	}
	if (fileName.empty()) {
		fileName = L"D:\\git\\NekoScript\\examples\\test.js";
	}

	JSProcess::_execPath = commandStrings[0];
	JSProcess::_scriptPath = fileName;
	for (unsigned int i = scriptArgvOffset; i < commandStrings.Size(); i++) {
		JSProcess::_scriptArgv.push_back(std::wstring(commandStrings[i]));
	}

	if (FileExists(fileName.c_str())) {
		duk_context *ctx = CreateDuktapeContext();
		duk_push_string(ctx, Utf16ToUtf8(fileName).c_str());
		if (duk_pcompile_string_filename(ctx, 0, ReadWholeFileAsString(fileName).c_str()) != DUK_EXEC_SUCCESS) {
			Fail(L"Compile Failed", Utf8ToUtf16(std::string(duk_safe_to_string(ctx, -1))));
			retCode = 1;
			duk_pop(ctx);
		} else {
			duk_int_t rc = duk_pcall(ctx, 0);
			if (rc != DUK_EXEC_SUCCESS) {
				Fail(L"Uncaught Error", Utf8ToUtf16(JSGetErrorStack(ctx)));
				retCode = 1;
				duk_pop(ctx);
			} else {
				duk_pop(ctx);
			}
		}

		duk_destroy_heap(ctx);
	} else {
		Fail(L"Module Not Found", fileName + L" cannot be found.");
	}

	return retCode;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	ShowConsole();
	isConsoleMode = false;
	return AppMain();
}

int main(int argc, char** argv) {
	isConsoleMode = true;
	return AppMain();
}