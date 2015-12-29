#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

class JSProcess {
private:
	JSProcess();
	~JSProcess();
	
	static void setupProperties(duk_context *ctx);
public:
	static void setup(duk_context *ctx);
	static void putOnStack(duk_context *ctx);

	static duk_ret_t pwd(duk_context *ctx);
	static duk_ret_t chdir(duk_context *ctx);
	static duk_ret_t abort(duk_context *ctx);
	static duk_ret_t exit(duk_context *ctx);

	static std::wstring _scriptPath;
	static std::vector<std::wstring> _scriptArgv;
	static std::wstring _execPath;
	static std::vector<std::wstring> _execArgv;
};

