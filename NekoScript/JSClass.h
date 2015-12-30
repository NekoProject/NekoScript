#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

#define JSClassIdentifier (JSClassIdentifier_(T))
#define JSClassIdentifier_(T) ((std::string("\xFFi") + (typeid(T).name() + 6)).c_str())

template<typename T>
class JSClass
{
	using PFunc = duk_ret_t(T::*)();
	using POperator = void(*)(duk_context *);

public:
	static void setup(duk_context *ctx) {
		duk_push_c_function(ctx, T::constructor, DUK_VARARGS);

		duk_push_string(ctx, typeid(T).name() + 6);
		duk_put_prop_string(ctx, -2, "name");

		T::setupPrototype(ctx);
	}

protected:
	duk_context *ctx;
	void *ptr;
	JSClass(duk_context *_ctx, void *_ptr) : ctx(_ctx), ptr(_ptr) {}

	template<PFunc func>
	static void registerMethod(duk_context *ctx, char* name, int nargs = 0) {
		duk_push_c_function(ctx, instanceMethodWrapper<func>, nargs);
		duk_push_string(ctx, (std::string(typeid(T).name() + 6) + "_" + name).c_str());
		duk_put_prop_string(ctx, -2, "name");
		duk_put_prop_string(ctx, -2, name);
	}

	static duk_ret_t constructorFunctionCall(duk_context *ctx) {
		return DUK_RET_TYPE_ERROR;
	}

	static void constructorPushThis(duk_context *ctx) {
		duk_push_this(ctx);
	}

	static void constructorExtra(duk_context *ctx, T* self) {}

	static duk_ret_t constructor(duk_context *ctx) {
		if (!duk_is_constructor_call(ctx)) {
			return T::constructorFunctionCall(ctx);
		}

		T::constructorPushThis(ctx);
		T* self = T::tryConstruct(ctx, duk_get_heapptr(ctx, -1));
		duk_push_c_function(ctx, finalizer, 1);
		duk_set_finalizer(ctx, -2);
		duk_push_pointer(ctx, reinterpret_cast<void*>(self));
		duk_put_prop_string(ctx, -2, JSClassIdentifier);

		T::constructorExtra(ctx, self);
		return 1;
	}

	static void finalizerExtra(duk_context *ctx, T* self) {}

	static duk_ret_t finalizer(duk_context *ctx) {
		T* self;
		duk_get_prop_string(ctx, -1, JSClassIdentifier);
		self = reinterpret_cast<T*>(duk_get_pointer(ctx, -1));
		duk_pop(ctx);
		T::finalizerExtra(ctx, self);
		duk_del_prop_string(ctx, -1, JSClassIdentifier);
		_ASSERTE(self);
		delete self;

		return 0;
	}

	static void operatorPushThis(duk_context *ctx) {
		duk_push_this(ctx);
	}

	static void operatorPushCurrentFunction(duk_context *ctx) {
		duk_push_current_function(ctx);
	}

	template<PFunc func, POperator oper = &T::operatorPushThis>
	static duk_ret_t instanceMethodWrapper(duk_context *ctx) {
		T* self;
		oper(ctx);
		duk_get_prop_string(ctx, -1, JSClassIdentifier);
		self = reinterpret_cast<T*>(duk_get_pointer(ctx, -1));
		_ASSERTE(self);
		duk_pop_2(ctx);

		return (self->*func)();
	}
};