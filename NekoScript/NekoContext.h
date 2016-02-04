#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>
#include <uv.h>

class NekoContext {
public:
	NekoContext();
	~NekoContext();

	duk_context *duk = nullptr;
	uv_loop_t *uv_loop = nullptr;

	static const NekoContext* fromDuktapeContext(duk_context *ctx);

private:
	duk_context *createDuktapeContext();
	void destroyDuktapeContext(duk_context *ctx);

	uv_loop_t *createUVLoop();
	void destroyUVLoop(uv_loop_t *uv_loop);

	static duk_ret_t initDuktapeContext(duk_context *ctx);
};

