#include "stdafx.h"
#include <duktape\duktape.h>
#include "NekoScript.h"
#include "Helper.h"
#include "JSEventEmitter.h"
#include "NKWinTaskDialog.h"

static int eval_raw(duk_context *ctx) {
	duk_compile(ctx, 0);/*
	duk_dump_function(ctx);
	void *ptr;
	duk_size_t sz;

	ptr = duk_require_buffer(ctx, -1, &sz);
	printf("buf=%p, size=%lu\n", ptr, (unsigned long)sz);
	FILE* test;
	fopen_s(&test, "d:\\a.bin", "wb");
	fwrite(ptr, sz, 1, test);
	fclose(test);
	duk_load_function(ctx);*/
	duk_call(ctx, 0);
	return 1;
}

static int tostring_raw(duk_context *ctx) {
	duk_to_string(ctx, -1);
	return 1;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	int x;
	duk_context *ctx;
	const char *res;
	AllocConsole();
	FILE* useless;
	freopen_s(&useless, "CONOUT$", "w", stdout);
	freopen_s(&useless, "CONOUT$", "w", stderr);
	freopen_s(&useless, "CONIN$", "r", stdin);

	ctx = duk_create_heap_default();

	JSEventEmitter::setup(ctx);
	NKWinTaskDialog::setup(ctx);
	duk_put_global_string(ctx, "NKWinTaskDialog");

	duk_push_string(ctx, ReadWholeFileAsString(L"D:\\git\\NekoScript\\examples\\test.js").c_str());
	duk_push_string(ctx, "test.js");
	duk_safe_call(ctx, eval_raw, 1 /*nargs*/, 1 /*nrets*/);
	duk_safe_call(ctx, tostring_raw, 1 /*nargs*/, 1 /*nrets*/);
	res = duk_get_string(ctx, -1);

	TaskDialog(NULL, hInstance,
		L"NekoScript",
		L"Result",
		Utf8ToUtf16(std::string(res)).c_str(),
		TDCBF_OK_BUTTON,
		TD_INFORMATION_ICON,
		&x);

	duk_pop(ctx);
	duk_destroy_heap(ctx);

	return 0;
}
