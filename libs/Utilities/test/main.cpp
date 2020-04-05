#include "Utilities.h"
#include "TimeStat.h"

#include <iostream>

int main(int, char**)
{
    ::std::wcout << utf8ToUtf16("Hello world") << ::std::endl;
    ::std::cout << utf16ToUtf8(L"Hello world") << ::std::endl;

    ::std::string utf8("utf8 string");
    ::std::wstring utf16;
    stringToWstring(utf8, utf16);
    ::std::wcout << utf16 << ::std::endl;

    utf16 = L"utf16 string";
    wstringToString(utf16, utf8);
    ::std::cout << utf8 << ::std::endl;
}