#include "JSProcess.h"
#include "Helper.h"

std::wstring JSProcess::_scriptPath;
std::vector<std::wstring> JSProcess::_scriptArgv;
std::wstring JSProcess::_execPath;
std::vector<std::wstring> JSProcess::_execArgv;

void JSProcess::setupProperties(duk_context *ctx) {
	duk_push_string(ctx, "win32");
	duk_put_prop_string(ctx, -2, "platform");

#ifdef __x86_64
	duk_push_string(ctx, "x64");
	duk_put_prop_string(ctx, -2, "arch");
#else
	duk_push_string(ctx, "ia32");
	duk_put_prop_string(ctx, -2, "arch");
#endif

	// env
	{
		duk_push_object(ctx);
		wchar_t *env = GetEnvironmentStrings();
		wchar_t *p = env;
		while (*p) {
			int length = wcslen(p);
			_ASSERTE(length > 0);

			wchar_t *pv = wcschr(p, L'=');
			_ASSERTE(pv != nullptr);
			_ASSERTE(pv - p >= 0);
			_ASSERTE(pv - p <= length - 1);

			std::wstring key = std::wstring(p, pv - p);
			std::wstring value = std::wstring(pv + 1, length - (pv - p) - 1);

			if (!key.empty()) {
				duk_push_string(ctx, Utf16ToUtf8(key).c_str());
				duk_push_string(ctx, Utf16ToUtf8(value).c_str());
				duk_put_prop(ctx, -3);
			}

			p += length + 1;
		}
		FreeEnvironmentStrings(env);
		duk_put_prop_string(ctx, -2, "env");
	}

	// argv
	{
		duk_push_array(ctx);
		int i = 0;
		duk_push_string(ctx, Utf16ToUtf8(_execPath).c_str());
		duk_put_prop_index(ctx, -2, i++);
		duk_push_string(ctx, Utf16ToUtf8(_scriptPath).c_str());
		duk_put_prop_index(ctx, -2, i++);
		for (const auto& value: _scriptArgv) {
			duk_push_string(ctx, Utf16ToUtf8(value).c_str());
			duk_put_prop_index(ctx, -2, i++);
		}
		duk_put_prop_string(ctx, -2, "argv");
	}

	// scriptArgv
	{
		duk_push_array(ctx);
		int i = 0;
		for (const auto& value : _scriptArgv) {
			duk_push_string(ctx, Utf16ToUtf8(value).c_str());
			duk_put_prop_index(ctx, -2, i++);
		}
		duk_put_prop_string(ctx, -2, "scriptArgv");
	}

	// execArgv
	{
		duk_push_array(ctx);
		int i = 0;
		for (const auto& value : _execArgv) {
			duk_push_string(ctx, Utf16ToUtf8(value).c_str());
			duk_put_prop_index(ctx, -2, i++);
		}
		duk_put_prop_string(ctx, -2, "execArgv");
	}

	duk_push_string(ctx, Utf16ToUtf8(_scriptPath).c_str());
	duk_put_prop_string(ctx, -2, "scriptPath");

	duk_push_string(ctx, Utf16ToUtf8(_execPath).c_str());
	duk_put_prop_string(ctx, -2, "execPath");

	duk_push_c_function(ctx, &pwd, 0);
	duk_push_string(ctx, "process_pwd");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "pwd");

	duk_push_c_function(ctx, &chdir, 1);
	duk_push_string(ctx, "process_chdir");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "chdir");

	duk_push_c_function(ctx, &exit, 1);
	duk_push_string(ctx, "process_exit");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "exit");

	duk_push_c_function(ctx, &abort, 0);
	duk_push_string(ctx, "process_abort");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "abort");
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

duk_ret_t JSProcess::pwd(duk_context *ctx) {
	DWORD result;
	{
		int size = MAX_PATH;
		wchar_t* buffer = new wchar_t[size];
		result = GetCurrentDirectory(size, buffer);

		if (result > size) {
			delete[] buffer;
			size = result;
			buffer = new wchar_t[size];
			result = GetCurrentDirectory(size, buffer);
		}

		if (result) {
			duk_push_string(ctx, Utf16ToUtf8(buffer).c_str());
		}

		delete[] buffer;
	}
	if (result) {
		return 1;
	}

	JSThrowWin32Error(ctx, "GetCurrentDirectory");
}

duk_ret_t JSProcess::chdir(duk_context *ctx) {
	DWORD result;
	{
		const char * path = duk_require_string(ctx, 0);
		result = SetCurrentDirectory(Utf8ToUtf16(path).c_str());
	}
	if (result) {
		return 0;
	}
	
	JSThrowWin32Error(ctx, "SetCurrentDirectory");
}

duk_ret_t JSProcess::abort(duk_context *ctx) {
	::abort();
}

duk_ret_t JSProcess::exit(duk_context *ctx) {
	::exit(duk_get_int(ctx, 0));
}
