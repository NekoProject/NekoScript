#include "stdafx.h"
#include "Helper.h"
#include "NKWinAPI.h"

unsigned short NKWinAPI::lastMagic = 0u;
std::map<const unsigned short, NKWinAPI*> NKWinAPI::magics = std::map<const unsigned short, NKWinAPI*>();

NKWinAPI::NKWinAPI(duk_context *ctx, void *ptr) : JSClass(ctx, ptr) {

}

NKWinAPI::~NKWinAPI() {}

void NKWinAPI::constructorPushThis(duk_context *ctx) {
	duk_push_c_function(ctx, callable, DUK_VARARGS);
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "NKWinAPIPrototype");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
	duk_set_prototype(ctx, -2);
}

void NKWinAPI::constructorExtra(duk_context *ctx, NKWinAPI* self) {
	duk_set_magic(ctx, -1, self->allocMagic());

	std::wstring moduleName = self->moduleName;
	std::size_t found = moduleName.find_last_of(L"/\\");
	if (found != std::wstring::npos) {
		moduleName = moduleName.substr(found + 1);
	}

	std::string name = Utf16ToUtf8(moduleName) + "!" + self->procName;
	duk_push_string(ctx, name.c_str());
	duk_put_prop_string(ctx, -2, "name");
}

void NKWinAPI::finalizerExtra(duk_context *ctx, NKWinAPI* self) {
	self->freeMagic();
}

unsigned short NKWinAPI::allocMagic() {
	_ASSERTE(!magicAllocated);

	magic = lastMagic + 1;
	while (magics.find(magic) != magics.end()) magic++;

	lastMagic = magic;
	magics[magic] = this;
	magicAllocated = true;
	return magic;
}

void NKWinAPI::freeMagic() {
	_ASSERTE(magicAllocated);
	magics.erase(magic);
	magicAllocated = false;
}

duk_ret_t NKWinAPI::callable(duk_context *ctx) {
	unsigned short magic = ((unsigned int)duk_get_current_magic(ctx)) & 0xffffU;
	NKWinAPI* self = magics[magic];
	return self->Call();
}

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
	obj->moduleName = GetModulePath(module);
	if (obj->moduleName.empty()) {
		obj->moduleName = Utf8ToUtf16(duk_require_string(ctx, 0));
	}
	obj->procName = duk_require_string(ctx, 1);
	obj->module = module;
	obj->proc = proc;
	return obj;
}

static unsigned int doStdcall(FARPROC proc, int argc, void* argv[]) {
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
		} else if (duk_is_string(ctx, i)) {
			value = (void*)duk_get_string(ctx, i);
		} else if ((value = duk_get_buffer_data(ctx, i, NULL))) {
		} else {
			delete[] args;
			duk_push_error_object(ctx, DUK_ERR_TYPE_ERROR, "Only integers, null's, undefined's, buffers and strings are supported. ");
			duk_throw(ctx);
		}
		
		*--p = value;
	}

	unsigned int ret = doStdcall(this->proc, count, args);
	delete[] args;
	duk_push_uint(ctx, ret);
	return 1;
}

void NKWinAPI::setupPrototype(duk_context *ctx) {
	duk_push_object(ctx);
	duk_get_prototype(ctx, 0);
	duk_set_prototype(ctx, -2);

	registerMethod<&NKWinAPI::Call>(ctx, "call2", DUK_VARARGS);

	duk_push_global_stash(ctx);
	duk_dup(ctx, -2);
	duk_put_prop_string(ctx, -2, "NKWinAPIPrototype");
	duk_pop(ctx);

	duk_put_prop_string(ctx, -2, "prototype");
}