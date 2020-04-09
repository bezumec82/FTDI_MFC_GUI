//#include "pch.h"
#include "ftdi.h"

using namespace FTDI;

void Writer::notifyAll(const EventCode& event,
	const Data& data)
{
	for (const auto& func : m_callBacks)
		func(event, data);
}

int32_t Writer::readFile()
{
	CFileException ex;
	if (m_fileName.IsEmpty())
	{
		::std::cerr << "File to write is not set" << ::std::endl;
		return -1;
	}
	if (!m_sendFile.Open(m_fileName,
		CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &ex))
	{
		::std::cout << "Can't open file "
			<< utf16ToUtf8(m_fileName.GetString()) << ::std::endl;

		return -1;
	}
	m_fileDataBuf.clear();
	m_fileDataBuf.resize(UINT(m_sendFile.GetLength()));
	UINT bytesRead = m_sendFile.Read(\
		m_fileDataBuf.data(), UINT(m_sendFile.GetLength()));
	m_sendFile.Close();
	if (bytesRead > 0)
	{
		::std::cout << "Data from file '"
			<< utf16ToUtf8(m_fileName.GetString())
			<< "' is stored." << ::std::endl;
		return 0;
	}
	else
	{
		::std::cerr << "No data was read" << ::std::endl;
		return -1;
	}
}

void Writer::setPeriod(CString& period)
{
	try
	{
		m_period = ::std::stoi(period.GetString());
		::std::cout << "Period is set to " << m_period << ::std::endl;
	}
	catch (const ::std::invalid_argument& ia)
	{
		::std::cerr << ia.what() << '\n';
		::std::cerr << "Invalid send period value" << ::std::endl;
		m_period = -1;
	}
}

void Writer::stop()
{
	m_startStopFlag.store(false);
	notifyAll(EventCode::STOPPED, Data{});
}

void Writer::sendOnce()
{
	::std::cout << "Sending once." << ::std::endl;
#if(0)
	if (m_ftdiHandler_ref.openSelDevice() != 0)
	{
		m_ftdiHandler_ref.closeSelDevice();
		notifyAll(EventCode::FTDI_OPEN_ERR, Data{});
		return;
	}
#endif
	m_ftdiHandler_ref.sendData(m_fileDataBuf);
#if(0)
	m_ftdiHandler_ref.closeSelDevice();
#endif
	notifyAll(EventCode::STOPPED, Data{});
	return;
}

void Writer::doSend()
{
	auto work = [&]() mutable {
#if(0)
		//try to open device
		if (m_ftdiHandler_ref.openSelDevice() != 0)
		{
			m_ftdiHandler_ref.closeSelDevice(); //try to fix situation
			notifyAll(EventCode::FTDI_OPEN_ERR, Data{});
			return;
		}
#endif
		::std::cout << "Start sending data to the device "
			<< m_ftdiHandler_ref.getSelDev() << ::std::endl;
		m_startStopFlag.store(true);
		while (m_startStopFlag.load() == true)
		{
			if (m_ftdiHandler_ref.sendData(m_fileDataBuf) != 0)
				break;
			::std::this_thread::sleep_for(::std::chrono::milliseconds(m_period));
		}
		m_startStopFlag.store(false);
#if(0)
		m_ftdiHandler_ref.closeSelDevice();
#endif
		notifyAll(EventCode::STOPPED, Data{});
		::std::cout << "Data sending is stopped" << ::std::endl;
	};
	m_future = ::std::async(std::launch::async, work);
}

void Writer::start()
{
	if (m_period < 0)
	{
		::std::cerr << "Period isn't set.\n"
			<< "Can't start sending." << ::std::endl;
		notifyAll(EventCode::NO_PERIOD_ERR, Data{});
		return;
	}

	if (m_fileDataBuf.empty())
	{
		::std::cerr << "No data to send. "
			<< "Can't start sending." << ::std::endl;
		notifyAll(EventCode::NO_DATA_ERR, Data{});
		return;
	}
	if (m_period == 0)
	{
		sendOnce();
		return;
	}
	doSend();
}