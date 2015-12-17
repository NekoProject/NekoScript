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