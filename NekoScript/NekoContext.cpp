#include "NekoContext.h"
#include "JSProcess.h"
#include "JSEngineExtra.h"
#include "JSFileSystem.h"
#include "JSEventEmitter.h"
#include "NKWinAPI.h"
#include "NKWinVersionInfo.h"
#include "NKWinTaskDialog.h"
#include <duktape/duk_internal.h>

NekoContext::NekoContext() {
	duk = createDuktapeContext();
	uv_loop = createUVLoop();
}

NekoContext::~NekoContext() {
	if (duk) {
		destroyDuktapeContext(duk);
		duk = nullptr; 
	}

	if (uv_loop) { 
		destroyUVLoop(uv_loop);
		uv_loop = nullptr;
	}
}

const NekoContext* NekoContext::fromDuktapeContext(duk_context *ctx) {
	return (NekoContext*)*(void**)(ctx + 1);
}

duk_context *NekoContext::createDuktapeContext() {
	duk_context *ctx = duk_create_heap_default();

	void** magic = (void**)(ctx + 1);
	*magic = (void*)this;

	if (duk_safe_call(ctx, initDuktapeContext, 0, 1) != DUK_EXEC_SUCCESS) {
		duk_destroy_heap(ctx);
		throw new std::runtime_error(std::string(duk_safe_to_string(ctx, -1)));
	}
	duk_pop(ctx);

	return ctx;
}

duk_ret_t NekoContext::initDuktapeContext(duk_context *ctx) {
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

void NekoContext::destroyDuktapeContext(duk_context *ctx) {
	duk_destroy_heap(ctx);
}

uv_loop_t *NekoContext::createUVLoop() {
	uv_loop_t *uv_loop = new uv_loop_t;
	uv_loop_init(uv_loop);

	return uv_loop;
}

void NekoContext::destroyUVLoop(uv_loop_t *uv_loop) {
	uv_loop_close(uv_loop);
	delete uv_loop;
}