#include "stdafx.h"
#include "Helper.h"

std::wstring Utf8ToUtf16(const std::string &s)
{
	std::wstring ret;
	int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), NULL, 0);
	if (len > 0)
	{
		ret.resize(len);
		MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), const_cast<wchar_t*>(ret.c_str()), len);
	}
	return ret;
}

std::string Utf16ToUtf8(const std::wstring &s)
{
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, s.c_str(), s.length(), const_cast<char*>(ret.c_str()), len, NULL, NULL);
	}
	return ret;
}

std::string ReadWholeFileAsString(const std::wstring &fname)
{
	std::ifstream t(fname);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string buffer(size, ' ');
	t.seekg(0);
	t.read(&buffer[0], size);

	return buffer;
}

std::string JSGetPropString(duk_context *ctx, const std::string& name)
{
	std::string result;
	duk_get_prop_string(ctx, -1, name.c_str());
	result = duk_to_string(ctx, -1);
	duk_pop(ctx);
	return result;
}

std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index)
{
	std::string result;
	duk_get_prop_index(ctx, -1, arr_index);
	result = duk_to_string(ctx, -1);
	duk_pop(ctx);
	return result;
}

std::string JSGetPropString(duk_context *ctx, const std::string& name, bool* isNull)
{
	std::string result;
	duk_get_prop_string(ctx, -1, name.c_str());
	*isNull = !!duk_is_null_or_undefined(ctx, -1);
	result = duk_to_string(ctx, -1);
	duk_pop(ctx);
	return result;
}

std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index, bool* isNull)
{
	std::string result;
	duk_get_prop_index(ctx, -1, arr_index);
	*isNull = !!duk_is_null_or_undefined(ctx, -1);
	result = duk_to_string(ctx, -1);
	duk_pop(ctx);
	return result;
}

bool JSGetPropBoolean(duk_context *ctx, const std::string& name)
{
	bool result;
	duk_get_prop_string(ctx, -1, name.c_str());
	result = !!duk_to_boolean(ctx, -1);
	duk_pop(ctx);
	return result;
}

bool JSGetPropBoolean(duk_context *ctx, duk_uarridx_t arr_index)
{
	bool result;
	duk_get_prop_index(ctx, -1, arr_index);
	result = !!duk_to_boolean(ctx, -1);
	duk_pop(ctx);
	return result;
}
