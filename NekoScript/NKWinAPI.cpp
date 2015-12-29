#include "stdafx.h"
#include "Helper.h"
#include "NKWinAPI.h"

NKWinAPI::NKWinAPI(duk_context *ctx, void *ptr) : JSClass(ctx, ptr) {

}

NKWinAPI::~NKWinAPI() {}

static HMODULE LoadLibraryUtf8(const char * str) {
	std::wstring path = Utf8ToUtf16(str);
	return LoadLibrary(path.c_str());
}


NKWinAPI* NKWinAPI::tryConstruct(duk_context *ctx, void *ptr) {
	HMODULE module = LoadLibraryUtf8(duk_require_string(ctx, 0));
	if (!module) JSThrowWin32Error(ctx, "LoadLibrary");

	FARPROC proc = GetProcAddress(module, duk_require_string(ctx, 1));
	if (!proc) JSThrowWin32Error(ctx, "GetProcAddress");

	NKWinAPI *obj = new NKWinAPI(ctx, ptr);
	obj->module = module;
	obj->proc = proc;
	return obj;
}

static int doStdcall(FARPROC proc, int argc, void* argv[]) {
	int retv = 0;
	FARPROC _proc = proc;
	int _argc = argc;
	void** _argv = argv;

	__asm {
		push eax
		push ebx
		push ecx

		mov ebx, _argv
		mov ecx, _argc
	again:
		push [ebx]
		add ebx, 4
		loop again

		mov eax, _proc
		call eax

		mov retv, eax
		pop ecx
		pop ebx
		pop eax
	}
	return retv;
}

duk_ret_t NKWinAPI::Call() {
	int count = duk_get_top(ctx);

	void** args = new void*[count];
	void** p = args + count;
	for (int i = 0; i < count; i ++) {
		void* value = 0;
		if (duk_is_number(ctx, i)) {
			value = (void*) (unsigned int) duk_get_uint(ctx, i);
		} else if (duk_is_null_or_undefined(ctx, i)) {
			value = 0;
		} else if (duk_is_buffer(ctx, i)) {
			value = duk_get_buffer_data(ctx, i, NULL);
		} else if (duk_is_string(ctx, i)) {
			value = (void*) duk_get_string(ctx, i);
		} else {
			delete[] args;
			duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "Only integers, nulls, undefineds, buffers and strings are supported. ");
			duk_throw(ctx);
		}
		
		*--p = value;
	}

	int ret = doStdcall(this->proc, count, args);

	delete[] args;
	return 0;
}

void NKWinAPI::setupPrototype(duk_context *ctx) {
	duk_push_object(ctx);
	registerMethod<&NKWinAPI::Call>(ctx, "call", DUK_VARARGS);
	duk_put_prop_string(ctx, -2, "prototype");
}