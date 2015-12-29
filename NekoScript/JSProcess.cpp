#include "JSProcess.h"
#include "Helper.h"

static void setupProperties(duk_context *ctx) {
	duk_push_string(ctx, "win32");
	duk_put_prop_string(ctx, -2, "platform");

	// env
	duk_push_object(ctx);
	{
		wchar_t *env = GetEnvironmentStrings();
		wchar_t *p = env;
		while (*p) {
			int length = wcslen(p);
			_ASSERTE(length > 0);

			wchar_t *pv = wcschr(p, L'=');
			_ASSERTE(pv != nullptr);
			_ASSERTE(pv - p > 0);
			_ASSERTE(pv - p < length - 1);

			std::wstring key = std::wstring(p, pv - p);
			std::wstring value = std::wstring(pv + 1, length - (pv - p) - 1);

			duk_push_string(ctx, Utf16ToUtf8(key).c_str());
			duk_push_string(ctx, Utf16ToUtf8(value).c_str());
			duk_put_prop(ctx, -3);

			p += length + 1;
		}
		FreeEnvironmentStrings(env);
	}
	duk_put_prop_string(ctx, -2, "env");
}

void JSProcess::setup(duk_context *ctx) {
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "process");
	if (duk_is_object(ctx, -1)) {
		duk_pop_2(ctx);
		return;
	}

	duk_pop(ctx);
	duk_push_object(ctx);
	setupProperties(ctx);
	duk_put_prop_string(ctx, -2, "process");
	duk_pop(ctx);
	duk_push_global_object(ctx);
	putOnStack(ctx);
	duk_put_prop_string(ctx, -2, "process");
	duk_pop(ctx);
}

void JSProcess::putOnStack(duk_context *ctx) {
	duk_push_global_stash(ctx);
	duk_get_prop_string(ctx, -1, "process");
	duk_swap_top(ctx, -2);
	duk_pop(ctx);
}