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
		::std::wcout << "Data from file '"
			<< m_sendFile.GetFilePath().GetString()
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
	m_stopSending.store(true);
	while (isSending());
}

void Writer::sendOnce()
{
	::std::cout << "Sending once." << ::std::endl;
	m_sendingOnce.store(true);
	return;
}

void Writer::doSend()
{
	auto work = [&]() mutable {
		::std::cout << "Start sending data to the device "
			<< m_ftdiHandler_ref.getSelDev() << ::std::endl;
		m_isSending.store(true);
		while (m_stopSending.load() == false)
		{
			if (m_ftdiHandler_ref.sendFile(\
				m_fileName, m_stopSending) != 0) { break; }
			::std::this_thread::sleep_for(::std::chrono::milliseconds(m_period));
			if (m_sendingOnce.load())
			{
				m_sendingOnce.store(false);
				break;
			}
		}//end while - future destructor here
		m_isSending.store(false);
		notifyAll(EventCode::STOPPED, Data{});
		::std::cout << "Data sending is stopped" << ::std::endl;
		MessageBeep(MB_ABORTRETRYIGNORE);
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
	m_stopSending.store(false);
	if (m_period == 0) { sendOnce(); }
	doSend();
}