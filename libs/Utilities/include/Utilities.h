#pragma once

#include <codecvt>
#include <string>
#include <algorithm>

#include <atlbase.h>
#include <atlconv.h>


std::wstring utf8ToUtf16(const std::string& utf8Str);
std::string utf16ToUtf8(const std::wstring& utf16Str);
void wstringToString(const ::std::wstring& in, ::std::string& out);
void stringToWstring(const ::std::string& in, ::std::wstring& out);