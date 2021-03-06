#include "JSEventEmitter.h"
#include "Helper.h"
#include "Resource.h"

void JSEventEmitter::setup(duk_context *ctx)
{
#ifndef NEKO_MINIMAL
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "EventEmitter");
	if (duk_is_object(ctx, -1)) {
		duk_pop_2(ctx);
		return;
	}

	duk_pop(ctx);
	duk_push_external_buffer(ctx);
	duk_config_buffer(ctx, -1, (void *) NekoResGet(NEKORES_INIT_EVENT_EMITTER), NekoResLength(NEKORES_INIT_EVENT_EMITTER));
	duk_load_function(ctx);
	duk_call(ctx, 0);
	duk_get_prop_string(ctx, -1, "prototype");
	duk_get_prop_string(ctx, -1, "emit");
	duk_put_prop_string(ctx, -4, "EventEmitterEmit");
	duk_put_prop_string(ctx, -3, "EventEmitterPrototype");
	duk_put_prop_string(ctx, -2, "EventEmitter");
	duk_pop(ctx);
#endif
}

void JSEventEmitter::putOnStack(duk_context *ctx)
{
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "EventEmitter");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
}

void JSEventEmitter::putPrototypeOnStack(duk_context *ctx)
{
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "EventEmitterPrototype");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
}

void JSEventEmitter::putEmitOnStack(duk_context *ctx)
{
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "EventEmitterEmit");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
}
