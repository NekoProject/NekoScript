#include "JSEngineExtra.h"
#include "Helper.h"

void JSEngineExtra::setup(duk_context *ctx) {
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "Duktape");
	if (duk_is_object(ctx, -1)) {
		duk_pop_2(ctx);
		return;
	}

	duk_pop(ctx);
	duk_push_global_object(ctx);
	duk_get_prop_string(ctx, -1, "Duktape");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);

	duk_push_c_function(ctx, &compile, 3);
	duk_push_string(ctx, "engineExtra_compile");
	duk_put_prop_string(ctx, -2, "name");
	duk_push_uint(ctx, DUK_COMPILE_EVAL);
	duk_put_prop_string(ctx, -2, "flagEval");
	duk_push_uint(ctx, DUK_COMPILE_FUNCTION);
	duk_put_prop_string(ctx, -2, "flagFunction");
	duk_push_uint(ctx, DUK_COMPILE_STRICT);
	duk_put_prop_string(ctx, -2, "flagStrict");
	duk_put_prop_string(ctx, -2, "compile");

	duk_push_c_function(ctx, &dumpBytecode, 3);
	duk_push_string(ctx, "engineExtra_dumpBytecode");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "dumpBytecode");

	duk_push_c_function(ctx, &loadBytecode, 3);
	duk_push_string(ctx, "engineExtra_loadBytecode");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "loadBytecode");

	duk_put_prop_string(ctx, -2, "Duktape");
	duk_pop(ctx);
}

void JSEngineExtra::putOnStack(duk_context *ctx) {
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "Duktape");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
}

duk_ret_t JSEngineExtra::compile(duk_context *ctx) {
	duk_require_string(ctx, 0);
	duk_require_string(ctx, 1);
	duk_uint_t type = duk_require_uint(ctx, 2);
	duk_dup(ctx, 0);
	duk_dup(ctx, 1);
	duk_compile(ctx, type);
	return 1;
}

duk_ret_t JSEngineExtra::dumpBytecode(duk_context *ctx) {
	duk_dup(ctx, 0);
	duk_dump_function(ctx);
	return 1;
}

duk_ret_t JSEngineExtra::loadBytecode(duk_context *ctx) {
	duk_dup(ctx, 0);
	duk_load_function(ctx);
	return 1;
}