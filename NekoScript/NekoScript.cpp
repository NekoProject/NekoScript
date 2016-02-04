#include "stdafx.h"
#include "Helper.h"
#include "NekoScript.h"
#include "NekoContext.h"
#include "JSProcess.h"
#include "JSFileSystem.h"
#include "JSEngineExtra.h"
#include "ExceptionHandler.h"
#include <lzma\CPP\Common\CommandLineParser.h>

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

	NekoContext* neko;
	try {
		neko = new NekoContext();
	} catch (std::runtime_error &e) {
		Fail(L"Initialization Failure", Utf8ToUtf16(e.what()));
		return -1;
	}

	if (fileName.size() > 4 && (fileName.substr(fileName.size() - 4).compare(L".jsc") == 0 || fileName.substr(fileName.size() - 4).compare(L".bin") == 0)) {
		duk_push_string(neko->duk, Utf16ToUtf8(fileName).c_str());
		if (duk_safe_call(neko->duk, JSFileSystem::readFileSync, 1, 1) != DUK_EXEC_SUCCESS) {
			Fail(L"Bytecode Error", Utf8ToUtf16(std::string(duk_safe_to_string(neko->duk, -1))));
			return -1;
		}
		if (duk_safe_call(neko->duk, JSEngineExtra::loadBytecode, 1, 1) != DUK_EXEC_SUCCESS) {
			Fail(L"Bytecode Error", Utf8ToUtf16(std::string(duk_safe_to_string(neko->duk, -1))));
			return -1;
		}
	} else {
		duk_push_string(neko->duk, Utf16ToUtf8(fileName).c_str());
		if (duk_pcompile_string_filename(neko->duk, 0, ReadWholeFileAsString(fileName).c_str()) != DUK_EXEC_SUCCESS) {
			Fail(L"Source Error", Utf8ToUtf16(std::string(duk_safe_to_string(neko->duk, -1))));
			return -1;
		}
	}

	duk_int_t rc = duk_pcall(neko->duk, 0);
	if (rc != DUK_EXEC_SUCCESS) {
		Fail(L"Uncaught Error", Utf8ToUtf16(JSGetErrorStack(neko->duk)));
		return -1;
	}

	duk_pop(neko->duk);
	delete neko;
	
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