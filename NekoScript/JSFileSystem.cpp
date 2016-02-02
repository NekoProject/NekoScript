#include "JSFileSystem.h"
#include "Helper.h"

duk_ret_t JSFileSystem::readdirSync(duk_context *ctx) {
	duk_require_string(ctx, 0);
	JSThrowScope
	{
		int length = duk_get_length(ctx, 0);
		if (length > MAX_PATH - 1)
			JSThrowWin32Error(ctx, ERROR_FILENAME_EXCED_RANGE);

		WIN32_FIND_DATA findfiledata;
		DWORD result;
		HANDLE hFind = INVALID_HANDLE_VALUE;

		wchar_t fullpath[MAX_PATH];
		result = GetFullPathName(Utf8ToUtf16(duk_get_string(ctx, 0)).c_str(), MAX_PATH, fullpath, 0);
		if (result == 0 || result >= MAX_PATH) JSThrowScopeWin32ErrorLast("GetFullPathName");

		std::wstring fp(fullpath);
		int i = 0;
		duk_push_array(ctx);

		hFind = FindFirstFile((fp + L"\\*").c_str(), &findfiledata);
		if (hFind == INVALID_HANDLE_VALUE) {
			if (GetLastError() != ERROR_FILE_NOT_FOUND) JSThrowScopeWin32ErrorLast("FindFirstFile");
		} else {
			do {
				if ((findfiledata.cFileName[0] != '.')) {
					duk_push_string(ctx, Utf16ToUtf8(findfiledata.cFileName).c_str());
					duk_put_prop_index(ctx, -2, i++);
				}
			} while (FindNextFile(hFind, &findfiledata) != 0);

			if (GetLastError() != ERROR_NO_MORE_FILES) {
				JSThrowScopeWin32ErrorLast("FindNextFile");
			}
		}
	}
	JSThrowScopeEnd

	return 1;
}

duk_ret_t JSFileSystem::existsSync(duk_context *ctx) {
	bool result = FileExists(Utf8ToUtf16(duk_safe_to_string(ctx, 0)).c_str());
	duk_push_boolean(ctx, result);
	return 1;
}

duk_ret_t JSFileSystem::readFileSync(duk_context *ctx) {
	const char* fn = duk_safe_to_string(ctx, 0);
	std::wstring fname = Utf8ToUtf16(fn);
	if (!FileExists(fname.c_str())) {
		duk_error(ctx, DUK_ERR_API_ERROR, "File not found: %s", fn);
		return 0;
	}

	std::ifstream t(fname, std::ios::in | std::ios::binary);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	char *buffer = (char *)duk_push_fixed_buffer(ctx, size);
	t.seekg(0);
	t.read(buffer, size);

	return 1;
}

duk_ret_t JSFileSystem::writeFileSync(duk_context *ctx) {
	const char *ptr;
	duk_size_t sz;

	if (duk_is_string(ctx, 1)) {
		sz = duk_get_length(ctx, 1);
		ptr = duk_get_string(ctx, 1);
	} else {
		ptr = (char*)duk_require_buffer_data(ctx, 1, &sz);
	}

	const char* fn = duk_safe_to_string(ctx, 0);
	std::wstring fname = Utf8ToUtf16(fn);
	std::ofstream t(fname, std::ios::out | std::ios::binary);
	if (t) {
		t.write(ptr, sz);
		t.flush();
		t.close();
	} else {
		duk_error(ctx, DUK_ERR_API_ERROR, "Stream cannot be opened for write: %s", fn);
	}

	return 0;
}

duk_ret_t JSFileSystem::unlinkSync(duk_context *ctx) {
	const char* fn = duk_safe_to_string(ctx, 0);
	bool answer = DeleteFileA(fn);
	if (!answer)
		duk_error(ctx, DUK_ERR_API_ERROR, "Can't delete file: %s", fn);
	return 0;
}

duk_ret_t JSFileSystem::renameSync(duk_context* ctx) {
	const char* old_fn = duk_safe_to_string(ctx, 0);
	const char* new_fn = duk_safe_to_string(ctx, 1);
	bool answer = MoveFileA(old_fn, new_fn);
	if (!answer)
		duk_error(ctx, DUK_ERR_API_ERROR, "Can't move file: %s to %s", old_fn, new_fn);
	return 0;
}



void JSFileSystem::setup(duk_context *ctx) {
	duk_push_object(ctx);
	
	duk_push_c_function(ctx, &readdirSync, 1);
	duk_push_string(ctx, "fs_readdirSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "readdirSync");

	duk_push_c_function(ctx, &existsSync, 1);
	duk_push_string(ctx, "fs_existsSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "existsSync");

	duk_push_c_function(ctx, &readFileSync, 1);
	duk_push_string(ctx, "fs_readFileSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "readFileSync");

	duk_push_c_function(ctx, &writeFileSync, 2);
	duk_push_string(ctx, "fs_writeFileSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "writeFileSync");

	duk_push_c_function(ctx, &unlinkSync, 1);
	duk_push_string(ctx, "fs_unlinkSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "unlinkSync");

	duk_push_c_function(ctx, &renameSync, 2);
	duk_push_string(ctx, "fs_renameSync");
	duk_put_prop_string(ctx, -2, "name");
	duk_put_prop_string(ctx, -2, "renameSync");

	duk_put_global_string(ctx, "fs");
}

