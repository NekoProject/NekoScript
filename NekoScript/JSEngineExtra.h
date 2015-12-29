#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

class JSEngineExtra {
private:
	JSEngineExtra();
	~JSEngineExtra();

public:
	static void setup(duk_context *ctx);
	static void putOnStack(duk_context *ctx);

	static duk_ret_t JSEngineExtra::compile(duk_context *ctx);
	static duk_ret_t JSEngineExtra::dumpBytecode(duk_context *ctx);
	static duk_ret_t JSEngineExtra::loadBytecode(duk_context *ctx);
};

