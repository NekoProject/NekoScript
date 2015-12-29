#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

class JSProcess {
private:
	JSProcess();
	~JSProcess();

public:
	static void setup(duk_context *ctx);
	static void putOnStack(duk_context *ctx);
};

