#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

class JSFileSystem {
private:
	JSFileSystem();
	~JSFileSystem();

public:
	static duk_ret_t readdirSync(duk_context *ctx);
	static duk_ret_t existsSync(duk_context *ctx);
	static duk_ret_t readFileSync(duk_context *ctx);
	static duk_ret_t writeFileSync(duk_context *ctx);
	static duk_ret_t unlinkSync(duk_context *ctx);
	static duk_ret_t renameSync(duk_context *ctx);

	static void setup(duk_context *ctx);
};

