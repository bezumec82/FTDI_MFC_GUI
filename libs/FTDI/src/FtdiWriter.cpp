#include "ftdi.h"

using namespace FTDI;

void Writer::registerCallBack(::std::function<CallBack> call_back)
{
	m_callBacks.emplace_back(call_back);
}

void Writer::notifyAll(const EventCode& event,
	const Data& data)
{

	for (const auto& func : m_callBacks)
		func(event, data);

}

INT Writer::openFile(CString& file_name, CFile& file)
{
	CFileException ex;
	if (file_name.IsEmpty())
	{
		::std::cerr << "File to send is not set" << ::std::endl;
		return -1;
	}
	if (!file.Open(file_name,
		CFile::modeRead
		| CFile::shareDenyNone, &ex))
	{
		TCHAR szCause[255] = { 0 };
		ex.GetErrorMessage(szCause, sizeof(szCause) / 2 - 1);
		::std::wcout << "Can't open file "
			<< file.GetFilePath().GetString() << '\n'
			<< "Error : " << szCause << ::std::endl;
		return -1;
	}
	::std::wcout << "File " << file.GetFilePath().GetString()
		<< " was opened" << ::std::endl;
	return 0;
}

INT Writer::readFile()
{
	CFile file;
	if (openFile(m_fileName, file) != 0) { return -1; }
	m_fileDataBuf.clear();
	m_fileDataBuf.resize(UINT(file.GetLength()));
	UINT bytesRead = file.Read(\
		m_fileDataBuf.data(), UINT(file.GetLength()));
	if (bytesRead > 0)
	{
		::std::wcout << "Data from file '"
			<< file.GetFilePath().GetString()
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
	m_stopSend.store(true);
	while (isSending());
}

void Writer::sendOnce()
{
	::std::cout << "Sending once." << ::std::endl;
	m_sendingOnce.store(true);
	return;
}

void Writer::rewindFile(CFile& file)
{
	file.Seek(0, CFile::begin);
}


void Writer::doSend()
{
	auto work = [&]() mutable {
		CFile file;
		TimeStat time_stat;
		::std::cout << "Start sending data thread. "
			<< "Device " << m_ftdiHandler_ref.getSelDev() << ::std::endl;
		if (openFile(m_fileName, file) != 0) {
			this->notifyAll(EventCode::WRITE_FOPEN_ERR, Data{});
			goto fileOpenError;
		}
		m_isSending.store(true);
		time_stat.start();
		while (m_stopSend.load() == false)
		{
			rewindFile(file);
			LONG sentBytes = m_ftdiHandler_ref.sendFile(\
				file, m_stopSend);
			if (sentBytes < 0) { break; }
			::std::this_thread::sleep_for(::std::chrono::milliseconds(m_period));
			if (m_sendingOnce.load())
			{
				m_sendingOnce.store(false);
				break;
			}
			time_stat.reportStream(sentBytes);
			m_ftdiHandler_ref.notifyAll(EventCode::MEDIUM_TX_RATE,
				Data{ time_stat.getMedByteRate() });
		}//end while - future destructor here
		m_isSending.store(false);
	fileOpenError:
		notifyAll(EventCode::WRITE_STOPPED, Data{});
		::std::cout << "Data sending is stopped" << ::std::endl;
		MessageBeep(MB_ABORTRETRYIGNORE);
		//CFile closed here
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
	m_stopSend.store(false);
	if (m_period == 0) { sendOnce(); }
	doSend();
}