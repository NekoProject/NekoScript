#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

class JSEventEmitter
{
private:
	JSEventEmitter();
	~JSEventEmitter();

public:
	static void setup(duk_context *ctx);
	static void putOnStack(duk_context *ctx);
	static void putPrototypeOnStack(duk_context *ctx);
	static void putEmitOnStack(duk_context *ctx);
};

