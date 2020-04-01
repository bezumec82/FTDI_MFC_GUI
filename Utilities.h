#pragma once

#include <codecvt>
#include <string>

std::wstring utf8ToUtf16(const std::string& utf8Str);
std::string utf16ToUtf8(const std::wstring& utf16Str);