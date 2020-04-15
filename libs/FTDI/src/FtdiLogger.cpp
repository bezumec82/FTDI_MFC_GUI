//#include "pch.h"
#include "ftdi.h"

using namespace FTDI;

INT Logger::openFile()
{
    if (m_fileName.IsEmpty())
    {
        ::std::cerr << "File to save is not set" << ::std::endl;
		m_ftdiHandler_ref.notifyAll(EventCode::WRITE_NO_FNAME_ERR, Data{} );
        return -1;
    }
    CFileException ex;
    if (!m_saveFile.Open(m_fileName,
        CFile::modeWrite
        | CFile::modeCreate
		| CFile::modeNoTruncate
        | CFile::shareDenyWrite, &ex))
    {
		TCHAR szCause[255] = { 0 };
		ex.GetErrorMessage(szCause, sizeof(szCause) / 2 - 1);
		::std::wcout << "Can't create/open file "
			<< m_saveFile.GetFilePath().GetString() << '\n'
			<< "Error : " << szCause << ::std::endl;
		m_ftdiHandler_ref.notifyAll(EventCode::READ_FOPEN_ERR, Data{} );
        return -1;
    }
    ::std::wcout << "File " << m_saveFile.GetFilePath().GetString()
        << " was successfully created/opened" << ::std::endl;
	m_fileOpenedFlag.store(true);
	//Seek to the end
	m_saveFile.SeekToEnd();
    return 0;
}

INT Logger::openDevice()
{
	FT_STATUS ftdi_stat = FT_OpenEx(
		m_node_ref.SerialNumber,
		FT_OPEN_BY_SERIAL_NUMBER,
		&m_node_ref.ftHandle);
	if (ftdi_stat != FT_OK)
	{
		::std::cout << "Can't open device : " << m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
		return -1;
	}
	else
	{
		::std::cout << "Opened device : " << m_devDescription << ::std::endl;
	}
	ftdi_stat = FT_SetTimeouts(m_node_ref.ftHandle, RX_TIMEOUT_MS, 0);
	if (ftdi_stat != FT_OK)
	{
		::std::cout << "Can't set RX timeout : "
			<< m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
		return -1;
	}
	return 0;
}

void Logger::closeDevice()
{
	FT_STATUS ftdi_stat = FT_Close(m_node_ref.ftHandle);
	if (ftdi_stat != FT_OK)
	{
		::std::cerr << "Can't close device " << m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
	}
	else
	{
		::std::cout << "Closed device : " << m_devDescription << ::std::endl;
	}
}

INT Logger::clearRxBuf()
{
	FT_STATUS ftdi_stat = FT_Purge(m_node_ref.ftHandle, FT_PURGE_RX);
	if (ftdi_stat != FT_OK)
	{
		::std::cerr << "Can't clear RX buffer of the device " << m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
		return -1;
	}
	return 0;
}

LONG Logger::recvData(::std::vector<char>& buffer)
{
	DWORD EventDWord{ 0 };
	DWORD TxBytes{ 0 }; DWORD RxBytes{ 0 };
	DWORD BytesReceived{ 0 };

	if (m_node_ref.ftHandle == nullptr)
	{
		::std::cerr << "Device isn't opened" << ::std::endl;
		return -1;
	}

	FT_STATUS ftdi_stat = FT_GetStatus(m_node_ref.ftHandle, \
		& RxBytes, &TxBytes, &EventDWord);
	if (ftdi_stat != FT_OK)
	{
		::std::cerr << "Can't get status from device : "
			<< m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
		return -1;
	}
	if (RxBytes == 0) return 0;

	buffer.resize(RxBytes);
	ftdi_stat = FT_Read(m_node_ref.ftHandle, \
		buffer.data(), DWORD(buffer.size()), &BytesReceived);
	if (ftdi_stat != FT_OK)
	{
		::std::cerr << "Can't read data from the device "
			<< m_devDescription << '\n'
			<< "Error : " << ftdi_stat << ::std::endl;
		return -1;
	}
	if (BytesReceived != RxBytes)
	{
		::std::cerr << "Error : " << BytesReceived
			<< " bytes out of declared " << RxBytes
			<< " is received" << ::std::endl;
		return -1;
	}
	return BytesReceived;
}

void Logger::doLogging(::std::vector<char>& buffer)
{
	if (m_startStopLogging.load())
	{
		if (!m_fileOpenedFlag.load()) //open and write
		{
			if (openFile() == 0)
			{
				m_saveFile.Write(buffer.data(), buffer.size());
				m_saveFile.Flush();
			}
		}
		else //just write
		{
			m_saveFile.Write(buffer.data(), buffer.size());
			m_saveFile.Flush();
		}
	}
}

INT Logger::doReading()
{
	//define what to do
	auto work = [&]() mutable
	{
		TimeStat time_stat;
		time_stat.start();
		m_isReading.store(true);
		while (m_startStopReading.load())
		{
			::std::this_thread::sleep_for(\
				::std::chrono::milliseconds(SAVE_PERIOD_MS));
			::std::vector<char> buffer;
			if (recvData(buffer) < 0) { break; }
			if (buffer.empty()) continue;
			doLogging(buffer);
			time_stat.reportStream(buffer.size());
			if (m_isSelDev.load())
			{
				m_ftdiHandler_ref.notifyAll(EventCode::MEDIUM_RX_RATE,
					Data{ time_stat.getMedByteRate() });
			}
		}
		closeDevice();
		time_stat.stop();
		if (m_fileOpenedFlag.load())
		{
			m_saveFile.Flush();
			m_saveFile.Close();
		}
		m_ftdiHandler_ref.notifyAll(EventCode::READ_STOPPED, Data{});
		::std::cout << "Data saving is stopped" << ::std::endl;
		m_isReading.store(false);
	};

	//try to open device
	if (openDevice() == 0)
	{
		::std::cout << "Start reading data from the device "
			<< m_devDescription << ::std::endl;
	}
	else
	{
		goto failure;
	}

	if (clearRxBuf() != 0)
	{
		goto failure;
	}
	m_future = ::std::async(std::launch::async, work);
	return 0;

failure:
	closeDevice();
	if (m_fileOpenedFlag.load())
		m_saveFile.Close();
	m_ftdiHandler_ref.notifyAll(EventCode::READ_STOPPED, Data{});
	return -1;
}

INT Logger::startReading()
{
	m_startStopReading.store(true);
	if (doReading() != 0)
	{
		::std::cerr << "Device logging is not started" << ::std::endl;
		return -1;
	}
	return 0;
}

void Logger::stop()
{
	m_startStopReading.store(false);
	m_startStopLogging.store(false);
	m_ftdiHandler_ref.notifyAll(EventCode::READ_STOPPED, Data{});
	while (isReading()); //wait for stop
}