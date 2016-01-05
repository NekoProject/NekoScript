#include "stdafx.h"
#include "Helper.h"
#include "NKWinVersionInfo.h"

NKWinVersionInfo::NKWinVersionInfo(duk_context *ctx, void *ptr) : JSClass(ctx, ptr) {
}

NKWinVersionInfo::~NKWinVersionInfo() {
	delete[] verData;
}

NKWinVersionInfo* NKWinVersionInfo::tryConstruct(duk_context *ctx, void *ptr) {
	const char * filename = duk_require_string(ctx, 0);
	DWORD verHandle = NULL;
	DWORD verSize = NULL;
	LPSTR verData = nullptr;

	JSThrowScope
	{
		std::wstring file = Utf8ToUtf16(filename);
		verHandle = NULL;
		verSize = GetFileVersionInfoSize(file.c_str(), &verHandle);
		if (!verSize) JSThrowScopeWin32ErrorLast("GetFileVersionInfoSize");

		verData = new char[verSize];
		if (!GetFileVersionInfo(file.c_str(), verHandle, verSize, verData)) {
			delete[] verData; verData = nullptr;
			JSThrowScopeWin32ErrorLast("GetFileVersionInfo");
		}
	}
	JSThrowScopeEnd

	NKWinVersionInfo* obj = new NKWinVersionInfo(ctx, ptr);
	obj->file = Utf8ToUtf16(filename);
	obj->verHandle = verHandle;
	obj->verSize = verSize;
	obj->verData = verData;
	return obj;
}

void NKWinVersionInfo::constructorExtra(duk_context *ctx, NKWinVersionInfo* self) {
	duk_push_string(ctx, "parse");
	duk_call_prop(ctx, -2, 0);
	duk_pop(ctx);
}

void NKWinVersionInfo::setupPrototype(duk_context *ctx) {
	duk_push_object(ctx);

	duk_push_string(ctx, "(function(){return this;})");
	duk_eval(ctx);
	duk_put_prop_string(ctx, -2, "parse");

	registerProperty<&Raw>(ctx, "raw");

	duk_put_prop_string(ctx, -2, "prototype");
}

duk_ret_t NKWinVersionInfo::Raw() {
	duk_push_fixed_buffer(ctx, verSize);
	void *buf = duk_get_buffer_data(ctx, -1, nullptr);
	memcpy(buf, (void *)verData, verSize);
	return 1;
}