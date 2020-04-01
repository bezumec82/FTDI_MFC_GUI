#include "pch.h"
#include "OneLine.h"

/*-----------------*/
/*--- Utilities ---*/
/*-----------------*/

void OneLine::getPeriod()
{

	CString freqText;
	m_eBoxSendPeriod.GetWindowText(freqText);
	try
	{
		m_period = ::std::stoi(freqText.GetString());
		::std::cout << "Period is set to " << m_period << ::std::endl;
	}
	catch (const ::std::invalid_argument& ia)
	{
		::std::cerr << ia.what() << '\n';
		::std::cerr << "Invalid send period value" << ::std::endl;
		m_period = -1;
	}
}

int32_t OneLine::readFile()
{
	CFile file;
	CFileException ex;

	if (m_openedFPth.IsEmpty())
	{
		::std::cerr << "File to write is not set" << ::std::endl;
		return -1;
	}
	if ( !file.Open(m_openedFPth, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite, &ex) )
	{
		::std::cout << "Can't open file "
					<< utf16ToUtf8(m_openedFPth.GetString()) << ::std::endl;

		return -1;
	}
	m_fileDataBuf.clear();
	m_fileDataBuf.resize(UINT(file.GetLength()));
	UINT bytesRead = file.Read(m_fileDataBuf.data(), UINT(file.GetLength()));
	file.Close();
	if (bytesRead > 0)
	{
		::std::cout << "Data from file '" << utf16ToUtf8(m_openedFPth.GetString())
					<< "' is stored." << ::std::endl;
		return 0;
	}
	else
	{
		::std::cerr << "No data was read" << ::std::endl;
		return -1;
	}


}

void OneLine::toggleStateBox()
{
	if (m_sendState.load())
		m_eBoxSendState.SetWindowTextW(L"sending");
	else
		m_eBoxSendState.SetWindowTextW(L"stopped");
}

/*----------------*/
/*--- Handlers ---*/
/*----------------*/
void OneLine::OpenHndlr()
{
	CFileDialog dlgFile(TRUE);
	if (dlgFile.DoModal() == IDOK)
	{
		m_openedFPth = dlgFile.GetPathName();		
		if(!readFile())
			m_eBoxOpenedFPth.SetWindowTextW(m_openedFPth);
	}
}

void OneLine::StartStopHndlr()
{
	//handle stop
	if (m_sendState.load()) //send in process
	{
		m_sendState.store(false);
		return;
	}

	//handle start
	getPeriod();
	if (m_period < 0)
	{
		::std::cerr << "Period isn't set.\n"
					<< "Can't start sending." << ::std::endl;
		return;
	}

	if (m_fileDataBuf.empty())
	{
		::std::cerr << "No data to send.\n"
					<< "Can't start sending." << ::std::endl;
		return;
	}
	
	if (m_period == 0) //send once
	{
		::std::cout << "Sending once." << ::std::endl;
		if (m_ftdiHandler_ref.openDevice() != 0)
		{
			m_ftdiHandler_ref.closeDevice();
			return;
		}
		m_ftdiHandler_ref.sendData(m_fileDataBuf);
		m_ftdiHandler_ref.closeDevice();
		return;
	}

	auto work = [&]() mutable {
		//try to open device
		if (m_ftdiHandler_ref.openDevice() != 0) return;
		::std::cout << "Start sending data to the device "
			<< m_ftdiHandler_ref.getSelDev() << ::std::endl;
		m_sendState.store(true);
		toggleStateBox();
		while (m_sendState.load() == true)
		{
			m_ftdiHandler_ref.sendData(m_fileDataBuf);
			::std::this_thread::sleep_for(::std::chrono::milliseconds(m_period));
		}
		m_ftdiHandler_ref.closeDevice();
		::std::cout << "Data sending is stopped" << ::std::endl;
		toggleStateBox();
	};
#if(1)
	m_future = ::std::async(std::launch::async, work);
#else
	m_worker = ::std::move(::std::thread(work));
#endif
}