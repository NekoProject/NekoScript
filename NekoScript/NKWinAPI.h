#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"

class NKWinAPI
	: public JSClass<NKWinAPI> {
private:
	HMODULE module = 0;
	FARPROC proc = 0;
	std::wstring moduleName;
	std::string procName;

public:
	NKWinAPI(duk_context *ctx, void *ptr);
	~NKWinAPI();

	duk_ret_t Call();

	static void constructorPushThis(duk_context *ctx);
	static void constructorExtra(duk_context *ctx, NKWinAPI* self);
	static NKWinAPI* tryConstruct(duk_context *ctx, void *ptr);
	static void setupPrototype(duk_context *ctx);

	static duk_ret_t toWString(duk_context *ctx);
	static duk_ret_t fromWString(duk_context *ctx);
	static duk_ret_t toAString(duk_context *ctx);
	static duk_ret_t fromAString(duk_context *ctx);
	static duk_ret_t throwError(duk_context *ctx);
};