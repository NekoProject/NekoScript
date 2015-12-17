#include "stdafx.h"
#include <duktape\duktape.h>

std::wstring Utf8ToUtf16(const std::string &s);
std::string Utf16ToUtf8(const std::wstring &s);
std::string ReadWholeFileAsString(const std::wstring &fname);

std::string JSGetPropString(duk_context *ctx, const std::string& name);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index);
std::string JSGetPropString(duk_context *ctx, const std::string& name, bool* isNull);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index, bool* isNull);
bool JSGetPropBoolean(duk_context *ctx, const std::string& name);
bool JSGetPropBoolean(duk_context *ctx, duk_uarridx_t arr_index);