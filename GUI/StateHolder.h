#pragma once

#include <string>
#include <vector>
#include <atomic>
#include <iostream>

#include "rapidjson/writer.h"
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"

#include "OneLine.h"
#include "Utilities.h"

#define FILE_NAME L"state.json"

class StateHolder
{
    using JsonBuffer = ::rapidjson::GenericStringBuffer<::rapidjson::UTF16<>>;
    using JsonStringValue =
        ::rapidjson::GenericValue<
            ::rapidjson::UTF16<>>;
#if(0)
    using JsonWriter = ::rapidjson::Writer<
        ::rapidjson::GenericStringBuffer<::rapidjson::UTF16<>>,
            ::rapidjson::UTF16<>, ::rapidjson::UTF16<>>;
#else
    using JsonWriter = ::rapidjson::PrettyWriter<
        ::rapidjson::GenericStringBuffer<::rapidjson::UTF16<>>,
        ::rapidjson::UTF16<>, ::rapidjson::UTF16<>>;
#endif
    using JsonDocument = ::rapidjson::GenericDocument<::rapidjson::UTF16<>>;

private: /*--- Nested facilities ---*/
    struct State
    {

    };

public: /*--- Construction ---*/
    StateHolder();

public: /*--- Methods ---*/
    void registerWriter(UINT, ::OneLine::View&);
    void saveState();
    void restoreState();

private: /*--- Implementation ---*/
    ::std::wstring writerKey(UINT);
    int32_t openFile(CString);
    int32_t readFile();

private: /*--- Variables ---*/
    ::std::unordered_map< UINT, ::OneLine::View& > m_viewMap;

    CString m_openedLogFile;

    CFile m_stateFile;
    CString m_jsonData;
};

