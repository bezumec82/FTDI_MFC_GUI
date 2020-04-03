#include "pch.h"
#include "ftdi.h"

using namespace FTDI;

int32_t Logger::openFile()
{
    if (m_fileName.IsEmpty())
    {
        ::std::cerr << "File to save is not set" << ::std::endl;
		notifyAll(EventCode::NO_FILE_NAME_ERR, Data{} );
        return -1;
    }
    CFileException ex;
	m_saveFile.Abort();
    if (!m_saveFile.Open(m_fileName,
        CFile::modeWrite
        | CFile::modeCreate
        | CFile::shareDenyWrite, &ex))
    {
        TCHAR szCause[255] = { 0 };
        ex.GetErrorMessage(szCause, sizeof(szCause) / 2 - 1);
        ::std::cout << "Can't create/open file "
            << utf16ToUtf8(m_fileName.GetString()) << '\n'
            << "Error : " << utf16ToUtf8(szCause)
            << ::std::endl;
        notifyAll(EventCode::FOPEN_ERR, Data{} );
        return -1;
    }
    ::std::cout << "File " << utf16ToUtf8(m_fileName.GetString())
        << " successfully created/opened" << ::std::endl;
    return 0;
}

void Logger::notifyAll(const EventCode& err_code, Data& data)
{
    for (const auto& func : m_callBacks)
        func(err_code, data);
}

void Logger::doLogging()
{
	auto work = [&]() mutable
	{
		TimeStat time_stat;
		//try to open device
		if (m_ftdiHandler_ref.openDevice() == 0)
		{
			::std::cout << "Start reading data from the device "
				<< m_ftdiHandler_ref.getSelDev() << ::std::endl;
		}
		else
		{
			m_ftdiHandler_ref.closeDevice(); //try to fix situation
			notifyAll(EventCode::STOPPED, Data{});
			return;
		}

		if (m_ftdiHandler_ref.clearRxBuf() != 0)
		{
			m_ftdiHandler_ref.closeDevice();
			notifyAll(EventCode::STOPPED, Data{});
			return;
		}
		time_stat.start();		
		m_isLogging.store(true);
		while (m_startStopFlag.load())
		{
			::std::vector<char> buffer;
			if (m_ftdiHandler_ref.recvData(buffer) != 0)
			{
				break;
			}
			m_saveFile.Write(buffer.data(), buffer.size());
			m_saveFile.Flush();

			notifyAll(EventCode::IMMEDIATE_RX_RATE,
				Data{time_stat.getImmRXrate(buffer.size())});
			notifyAll(EventCode::MEDIUM_RX_RATE,
				Data{time_stat.getMedRXrate(buffer.size())});
			::std::this_thread::sleep_for(\
				::std::chrono::milliseconds(SAVE_PERIOD_MS));
		}
		m_ftdiHandler_ref.closeDevice();
		time_stat.stop();
		m_saveFile.Flush();
		m_saveFile.Close();		
		notifyAll(EventCode::STOPPED, Data{});
		::std::cout << "Data saving is stopped" << ::std::endl;
		m_isLogging.store(false);
	};

	m_future = ::std::async(std::launch::async, work);
}

void Logger::start()
{
	if (!openFile())
	{
		m_startStopFlag.store(true);
		doLogging();
	}
}
void Logger::stop()
{
    m_startStopFlag.store(false);
	notifyAll(EventCode::STOPPED, Data{});
}