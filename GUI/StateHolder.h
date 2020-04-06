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

public: /*--- Construction ---*/
    StateHolder()
    {
        if (openFile(FILE_NAME) != 0) return;
        if (readFile() <= 0) return;
        //const TCHAR json[] = L" { \"Writer #0\": \"H:/PROJECTS/FTDI/KTV_Cmd/SET_N15_RGBall_#0000FF.uio\" } "; //test
        //if (m_document.Parse(json).HasParseError())
        if (m_document.Parse(m_jsonData.GetString()).HasParseError())
        {
            ::std::cerr << "Can't parse input data" << std::endl;
        }
        else
        {
            ::std::cout << "Document was successfully parsed" << ::std::endl;
        }
    }

public: /*--- Methods ---*/
    void registerWriter(UINT line_idx, ::OneLine::View& view)
    {
        m_viewMap.insert({ line_idx, view });
    }

    void saveState();
    void restoreState();

private: /*--- Implementation ---*/
    ::std::wstring writerKey(UINT line_idx)
    {
        return ::std::wstring(L"Writer #")
            + ::std::to_wstring(line_idx);
    }

    int32_t openFile(CString);
    int32_t readFile();

private: /*--- Variables ---*/
    ::rapidjson::GenericReader<::rapidjson::UTF16<>, ::rapidjson::UTF16<>> m_reader;
    ::rapidjson::GenericDocument<::rapidjson::UTF16<>> m_document;

    ::std::unordered_map< UINT, ::OneLine::View& > m_viewMap;
    CString m_openedLogFile;

    CFile m_stateFile;
    CString m_jsonData;
};

