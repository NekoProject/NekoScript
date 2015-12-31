#pragma once
#include "stdafx.h"
#include <duktape\duktape.h>

std::wstring Utf8ToUtf16(const std::string &s);
std::string Utf16ToUtf8(const std::wstring &s);
std::wstring ANSIToUtf16(const std::string &s);
std::string Utf16ToANSI(const std::wstring &s);
std::string ReadWholeFileAsString(const std::wstring &fname);
bool FileExists(LPCTSTR szPath);
std::wstring GetModulePath(HMODULE module);

std::string JSGetPropString(duk_context *ctx, const std::string& name);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index);
std::string JSGetPropString(duk_context *ctx, const std::string& name, bool* isNull);
std::string JSGetPropString(duk_context *ctx, duk_uarridx_t arr_index, bool* isNull);
bool JSGetPropBoolean(duk_context *ctx, const std::string& name);
bool JSGetPropBoolean(duk_context *ctx, duk_uarridx_t arr_index);
std::string JSReplaceString(duk_context *ctx, const std::string& str, const std::string& pattern, const std::string& replacement, const std::string& option = "g");
std::string JSGetErrorStack(duk_context *ctx);

void JSPushWin32ErrorObject(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function);
__declspec(noreturn) void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, const char * function = 0);
__declspec(noreturn) void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function = 0);

#define JSThrowWin32Error(...)  \
	JSThrowWin32ErrorRaw((const char *) (__FILE__), (duk_int_t) (__LINE__), __VA_ARGS__)
