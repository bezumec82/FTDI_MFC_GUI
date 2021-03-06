#include "Utilities.h"

std::wstring utf8ToUtf16(const std::string& utf8Str)
{
    return (std::wstring{ CA2W{ utf8Str.c_str() } } );
}

std::string utf16ToUtf8(const std::wstring& utf16Str)
{
    return ( std::string{ CW2A{ utf16Str.c_str() } });
}

void wstringToString(const ::std::wstring& in, ::std::string& out)
{
    ::std::string ret_val(in.begin(), in.end());
    out.swap(ret_val);
}

void stringToWstring(const ::std::string& in, ::std::wstring& out)
{
    ::std::wstring ret_val(in.begin(), in.end());
    out.swap(ret_val);
}