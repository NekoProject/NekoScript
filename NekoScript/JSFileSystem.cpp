#include "JSFileSystem.h"
#include "Helper.h"

duk_ret_t JSFileSystem::readdirSync(duk_context *ctx) {
	return 0;
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

void JSFileSystem::setup(duk_context *ctx) {
	duk_push_object(ctx);
	
	//duk_push_c_function(ctx, &readdirSync, 1);
	//duk_push_string(ctx, "fs_readdirSync");
	//duk_put_prop_string(ctx, -2, "name");
	//duk_put_prop_string(ctx, -2, "readdirSync");

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

	duk_put_global_string(ctx, "fs");
}

