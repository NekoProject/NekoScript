#pragma once
#include "stdafx.h"
#include "JSClass.h"
#include "Helper.h"

class NKWinVersionInfo
	: public JSClass<NKWinVersionInfo> {
private:
	std::wstring file;
	DWORD verHandle = NULL;
	DWORD verSize = NULL;
	LPSTR verData = nullptr;

public:
	NKWinVersionInfo(duk_context *ctx, void *ptr);
	~NKWinVersionInfo();

	duk_ret_t Raw();

	static NKWinVersionInfo* tryConstruct(duk_context *ctx, void *ptr);
	static void constructorExtra(duk_context *ctx, NKWinVersionInfo* self);
	static void setupPrototype(duk_context *ctx);
};

