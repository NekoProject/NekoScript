#pragma once
#include <duktape\duktape.h>
#include "stdafx.h"

template<typename T>
class JSClass
{
	using PFunc = duk_ret_t(T::*)();
public:
	static void setup(duk_context *ctx) {
		duk_push_c_function(ctx, constructor, 0);
		duk_push_object(ctx);
		T::setupPrototype(ctx);
		duk_put_prop_string(ctx, -2, "prototype");
	}

protected:
	duk_context *ctx;
	void *ptr;
	JSClass(duk_context *_ctx, void *_ptr) : ctx(_ctx), ptr(_ptr) {}

	template<PFunc func>
	static void registerMethod(duk_context *ctx, char* name, int nargs = 0) {
		duk_push_c_function(ctx, instanceMethodWrapper<func>, nargs);
		duk_put_prop_string(ctx, -2, name);
	}

	static duk_ret_t constructor(duk_context *ctx) {
		if (!duk_is_constructor_call(ctx)) {
			return DUK_RET_TYPE_ERROR;
		}

		duk_push_this(ctx);
		T* self = new T(ctx, duk_get_heapptr(ctx, -1));
		duk_push_c_function(ctx, finalizer, 1);
		duk_set_finalizer(ctx, -2);
		duk_push_pointer(ctx, reinterpret_cast<void*>(self));
		duk_put_prop_string(ctx, -2, "\xFF" "i");

		return 0;
	}

	static duk_ret_t finalizer(duk_context *ctx) {
		T* self;
		duk_get_prop_string(ctx, -1, "\xFF" "i");
		self = reinterpret_cast<T*>(duk_get_pointer(ctx, -1));
		delete self;

		return 0;
	}

	template<PFunc func>
	static duk_ret_t instanceMethodWrapper(duk_context *ctx) {
		T* self;
		duk_push_this(ctx);
		duk_get_prop_string(ctx, -1, "\xFF" "i");
		self = reinterpret_cast<T*>(duk_get_pointer(ctx, -1));
		duk_pop_2(ctx);

		return (self->*func)();
	}
};