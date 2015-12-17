#include "stdafx.h"

std::wstring Utf8ToUtf16(const std::string &s);
std::string Utf16ToUtf8(const std::wstring &s);
std::string ReadWholeFileAsString(const std::wstring &fname);