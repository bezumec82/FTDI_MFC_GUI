#include "pch.h"
#include "StateHolder.h"

int32_t StateHolder::openFile(CString file_path)
{
    CFileException ex;
    if (file_path.IsEmpty())
    {
        ::std::cerr << "File path set" << ::std::endl;
        return -1;
    }
    if (!m_stateFile.Open(file_path,
        CFile::modeReadWrite | CFile::modeCreate
        | CFile::modeNoTruncate | CFile::shareDenyWrite, &ex))
    {
        TCHAR szCause[255] = { 0 };
        ex.GetErrorMessage(szCause, sizeof(szCause) / 2 - 1);
        ::std::wcout << "Can't create/open file "
            << m_stateFile.GetFilePath() << '\n'
            << "Error : " << szCause
            << ::std::endl;
        return -1;
    }
    ::std::wcout << "File " << m_stateFile.GetFilePath().GetString() << " was opened" << ::std::endl;
    return 0;
}

int32_t StateHolder::readFile()
{
    UINT num_bytes = m_stateFile.GetLength();
    UINT num_chars = m_stateFile.GetLength() / sizeof(TCHAR);
    UINT read_bytes = m_stateFile.Read(m_jsonData.GetBuffer(num_chars), num_bytes);
    m_jsonData.ReleaseBuffer();
    if (read_bytes != num_bytes)
    {
        ::std::cerr << "Error reading file" << ::std::endl;
        return -1;
    }
    ::std::cout << read_bytes << " bytes was successfully read" << ::std::endl;
    return read_bytes;
}

void StateHolder::saveState()
{
    JsonBuffer json_buffer;
    JsonWriter json_writer(json_buffer);
    json_writer.StartObject();
    for (auto& val : m_viewMap)
    {
        ::std::wstring key = ::std::wstring(L"Writer #")
            + ::std::to_wstring(val.first);

        json_writer.Key(key.c_str());
        CString file_name{ val.second.getFile().GetString() };
        //Json doesn't support shitty '\\'
        file_name.Replace(L'\\', L'/');
        json_writer.String(file_name);
    }
    json_writer.Key(L"Logger");
    json_writer.String(m_openedLogFile.GetString());
    json_writer.Flush();
    json_writer.EndObject();

    m_stateFile.Seek(CFile::begin, 0);
    m_stateFile.Write(json_buffer.GetString(), json_buffer.GetSize());
    m_stateFile.Flush();
}

void StateHolder::restoreState()
{
    if (!m_document.IsObject()) return;
    for (auto& map_pair : m_viewMap)
    {
        //Get index from map and find according line in state file
        ::std::wstring key = writerKey(map_pair.first);

        if (m_document.FindMember(key.c_str()) != m_document.MemberEnd())
        {
            JsonStringValue& file_name = m_document[key.c_str()];
            map_pair.second.setFile(file_name.GetString());
        }
        else
        {
            map_pair.second.setFile(L"");
        }
    }
}