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

	bool magicAllocated = false;
	unsigned short magic = 0u;

	unsigned short allocMagic();
	void freeMagic();

	static unsigned short lastMagic;
	static std::map<const unsigned short, NKWinAPI*> magics;

public:
	NKWinAPI(duk_context *ctx, void *ptr);
	~NKWinAPI();

	duk_ret_t Call();

	static void constructorPushThis(duk_context *ctx);
	static void constructorExtra(duk_context *ctx, NKWinAPI* self);
	static void finalizerExtra(duk_context *ctx, NKWinAPI* self);
	static duk_ret_t callable(duk_context *ctx);
	static NKWinAPI* tryConstruct(duk_context *ctx, void *ptr);
	static void setupPrototype(duk_context *ctx);
};