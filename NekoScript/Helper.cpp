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

std::wstring ANSIToUtf16(const std::string &s) {
	std::wstring ret;
	int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length(), NULL, 0);
	if (len > 0) {
		ret.resize(len);
		MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.length(), const_cast<wchar_t*>(ret.c_str()), len);
	}
	return ret;
}

std::string Utf16ToANSI(const std::wstring &s) {
	std::string ret;
	int len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.length(), NULL, 0, NULL, NULL);
	if (len > 0) {
		ret.resize(len);
		WideCharToMultiByte(CP_ACP, 0, s.c_str(), s.length(), const_cast<char*>(ret.c_str()), len, NULL, NULL);
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

bool FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

std::wstring GetModulePath(HMODULE module) {
	std::vector<wchar_t> executablePath(MAX_PATH);

	DWORD result = GetModuleFileNameW(module, &executablePath[0], static_cast<DWORD>(executablePath.size()));
	while (result == executablePath.size()) {
		executablePath.resize(executablePath.size() * 2);
		result = GetModuleFileNameW(module, &executablePath[0], static_cast<DWORD>(executablePath.size()));
	}

	if (result == 0) {
		return std::wstring();
	}

	return std::wstring(executablePath.begin(), executablePath.begin() + result);
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

static int JSGetErrorStackInternal(duk_context *ctx) {
	duk_get_prop_string(ctx, -1, "stack");
	return 1;
}

std::string JSReplaceString(duk_context *ctx, const std::string& str, const std::string& pattern, const std::string& replacement, const std::string& option) {
	duk_int_t result;
	
	result = duk_peval_string(ctx, "(function(s,h,i,t){return s.replace(new RegExp(h,t),i)})");
	if (result != 0) {
		std::string msg = duk_safe_to_string(ctx, -1);
		duk_pop(ctx);

		throw std::runtime_error(msg);
	}

	duk_push_string(ctx, str.c_str());
	duk_push_string(ctx, pattern.c_str());
	duk_push_string(ctx, replacement.c_str());
	duk_push_string(ctx, option.c_str());
	
	result = duk_pcall(ctx, 4);
	if (result != 0) {
		std::string msg = duk_safe_to_string(ctx, -1);
		duk_pop(ctx);

		throw std::runtime_error(msg);
	}

	std::string output = duk_get_string(ctx, -1);
	duk_pop(ctx);
	return output;
}

std::string JSGetErrorStack(duk_context *ctx) {
	duk_dup_top(ctx);
	duk_safe_call(ctx, JSGetErrorStackInternal, 1, 1);
	duk_safe_to_string(ctx, -1);
	std::string result = duk_get_string(ctx, -1);
	duk_pop(ctx);
	return JSReplaceString(ctx, result, "^\\t", " *  ", "mg");
}

void JSPushWin32ErrorObject(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function) {
	LPVOID buffer;
	DWORD msg;

	msg = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&buffer,
		0, NULL);

	{
		std::string message;

		if (msg) {
			message = Utf16ToUtf8(std::wstring((LPCTSTR)buffer));
			LocalFree(buffer);
		}
		if (function) {
			if (msg)
				duk_push_error_object_raw(ctx, DUK_ERR_ERROR, filename, line, "Win32Error(0x%08x)@%s: %s", code, function, message.c_str());
			else
				duk_push_error_object_raw(ctx, DUK_ERR_ERROR, filename, line, "Win32Error(0x%08x)@%s", code, function);
		} else {
			if (msg)
				duk_push_error_object_raw(ctx, DUK_ERR_ERROR, filename, line, "Win32Error(0x%08x): %s", code, message.c_str());
			else
				duk_push_error_object_raw(ctx, DUK_ERR_ERROR, filename, line, "Win32Error(0x%08x)", code);
		}

		duk_push_uint(ctx, code);
		duk_put_prop_string(ctx, -2, "code");

		if (function)
			duk_push_string(ctx, function);
		else
			duk_push_undefined(ctx);
		duk_put_prop_string(ctx, -2, "function");
	}
}

void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, const char * function) {
	JSThrowWin32ErrorRaw(filename, line, ctx, GetLastError(), function);
}

void JSThrowWin32ErrorRaw(const char * filename, duk_int_t line, duk_context *ctx, DWORD code, const char * function) {
	JSPushWin32ErrorObject(filename, line, ctx, code, function);
	duk_throw(ctx);
}
