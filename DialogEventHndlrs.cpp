#include "pch.h"
#include "Dialog.h"

BEGIN_MESSAGE_MAP(CMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(IDSCAN, &CMFCDlg::OnBnClickedScan)
	ON_CBN_SELCHANGE(IDC_COMBO, &CMFCDlg::OnCbnSelchangeCombo)

	ON_COMMAND_RANGE(IDOPEN1, IDOPEN4, openDispatch)
	ON_COMMAND_RANGE(IDSTARTSTOP1, IDSTARTSTOP4, startStopDispatch)

	ON_BN_CLICKED(IDSAVE, &CMFCDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDSTARTSTOPSAVE, &CMFCDlg::OnBnClickedStartSave)
END_MESSAGE_MAP()

void CMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO, m_cBoxDevices);

	DDX_Control(pDX, IDC_EBOX_FILE1, oneLine1.m_eBoxOpenedFPth);
	DDX_Control(pDX, IDC_EBOX_FILE2, oneLine2.m_eBoxOpenedFPth);
	DDX_Control(pDX, IDC_EBOX_FILE3, oneLine3.m_eBoxOpenedFPth);
	DDX_Control(pDX, IDC_EBOX_FILE4, oneLine4.m_eBoxOpenedFPth);

	DDX_Control(pDX, IDC_EBOX_STATE1, oneLine1.m_eBoxSendState);
	DDX_Control(pDX, IDC_EBOX_STATE2, oneLine2.m_eBoxSendState);
	DDX_Control(pDX, IDC_EBOX_STATE3, oneLine3.m_eBoxSendState);
	DDX_Control(pDX, IDC_EBOX_STATE4, oneLine4.m_eBoxSendState);

	DDX_Control(pDX, IDC_PERIOD1, oneLine1.m_eBoxSendPeriod);
	DDX_Control(pDX, IDC_PERIOD2, oneLine2.m_eBoxSendPeriod);
	DDX_Control(pDX, IDC_PERIOD3, oneLine3.m_eBoxSendPeriod);
	DDX_Control(pDX, IDC_PERIOD4, oneLine4.m_eBoxSendPeriod);

	DDX_Control(pDX, IDC_EBOX_SAVE_FILE, m_eBoxSaveFPth);
	DDX_Control(pDX, IDC_EBOX_SAVE_STATE, m_eBoxSaveState);
}

void CMFCDlg::OnBnClickedScan()
{
	m_ftdiHandler.findFTDIDevices();
	m_ftdiHandler.printFTDIDevices();
	//fill combo box
	m_cBoxDevices.ResetContent();
	const ::FTDI::DevDescriptions& dev_descs = m_ftdiHandler.getDevDescriptions();
	for (const ::std::string& desc : dev_descs)
	{
		m_cBoxDevices.AddString(CString{ desc.c_str() });
	}
}

void CMFCDlg::OnCbnSelchangeCombo()
{
	CString sel_device;
	m_cBoxDevices.GetLBText(m_cBoxDevices.GetCurSel(), sel_device);
	m_ftdiHandler.setSelDev( utf16ToUtf8(sel_device.GetString()));
	UpdateData(FALSE);
}

void CMFCDlg::openDispatch(UINT nID)
{
	switch (nID)
	{
	case IDOPEN1:
	{
		::std::cout << "Open1 pressed" << ::std::endl;
		oneLine1.OpenHndlr();
		break;
	}
	case IDOPEN2:
	{
		::std::cout << "Open2 pressed" << ::std::endl;
		oneLine2.OpenHndlr();
		break;
	}
	case IDOPEN3:
	{
		::std::cout << "Open3 pressed" << ::std::endl;
		oneLine3.OpenHndlr();
		break;
	}
	case IDOPEN4:
	{
		::std::cout << "Open4 pressed" << ::std::endl;
		oneLine4.OpenHndlr();
		break;
	}
	default:
		::std::cerr << "Error : unhandled code : " << nID << ::std::endl;
	}
}

void CMFCDlg::startStopDispatch(UINT nID)
{
	switch (nID)
	{
	case IDSTARTSTOP1:
	{
		::std::cout << "Start/Stop1 pressed" << ::std::endl;
		oneLine1.StartStopHndlr();
		break;
	}
	case IDSTARTSTOP2:
	{
		::std::cout << "Start/Stop2 pressed" << ::std::endl;
		oneLine2.StartStopHndlr();
		break;
	}
	case IDSTARTSTOP3:
	{
		::std::cout << "Start/Stop3 pressed" << ::std::endl;
		oneLine3.StartStopHndlr();
		break;
	}
	case IDSTARTSTOP4:
	{
		::std::cout << "Start/Stop4 pressed" << ::std::endl;
		oneLine4.StartStopHndlr();
		break;
	}
	default:
		::std::cerr << "Error : unhandled code : " << nID << ::std::endl;
	}
}

void CMFCDlg::OnBnClickedSave()
{
	CFileDialog dlgFile(FALSE); //save file
	if (dlgFile.DoModal() == IDOK)
	{
		m_saveFPth = dlgFile.GetPathName();
		m_eBoxSaveFPth.SetWindowTextW(m_saveFPth);
	}
}

#define SAVE_PERIOD_MS 100
void CMFCDlg::OnBnClickedStartSave()
{
	CFileException ex;

	//handle stop
	if (m_saveState.load()) //reading/saving in process
	{
		m_saveState.store(false);
		return;
	}

	//open file
	if ( m_saveFPth.IsEmpty() )
	{
		::std::cerr << "File to save is not set" << ::std::endl;
		return;
	}
	if (!m_saveFile.Open(m_saveFPth,
		CFile::modeWrite | CFile::modeCreate | CFile::shareDenyWrite, &ex))
	{
		TCHAR szCause[255] = { 0 };
		ex.GetErrorMessage(szCause, sizeof(szCause)/2 - 1);
		::std::cout << "Can't create/open file "
			<< utf16ToUtf8(m_saveFPth.GetString()) << '\n'
			<< "Error : " << utf16ToUtf8(szCause)
			<< ::std::endl;
		return;
	}

	//reading to the file
	auto work = [&]() mutable
	{
		//try to open device
		if (m_ftdiHandler.openDevice() == 0)
		{
			::std::cout << "Start reading data from the device "
						<< m_ftdiHandler.getSelDev() << ::std::endl;
			m_saveState.store(true);
			m_eBoxSaveState.SetWindowTextW(L"saving");
		}
		else
		{
			m_ftdiHandler.closeDevice(); //try to fix situation
		}
		while (m_saveState.load())
		{
			::std::vector<char> buffer;
			m_ftdiHandler.recvData(buffer);
			m_saveFile.Write(buffer.data(), buffer.size());
			m_saveFile.Flush();
			::std::this_thread::sleep_for(::std::chrono::milliseconds(SAVE_PERIOD_MS));
		}
		m_eBoxSaveState.SetWindowTextW(L"stopped");
		m_ftdiHandler.closeDevice();
		m_saveFile.Close();
		::std::cout << "Data saving is stopped" << ::std::endl;
	};

	m_saveFuture = ::std::async(std::launch::async, work);
}


#if(0)
void CMFCDlg::OnBnClickedOpen1()
{
	setName(m_eBoxOpenedFPth1, m_openedFPth1);
}

void CMFCDlg::OnBnClickedOpen2()
{
	setName(m_eBoxOpenedFPth2, m_openedFPth2);
}

void CMFCDlg::OnBnClickedOpen3()
{
	setName(m_eBoxOpenedFPth3, m_openedFPth3);
}

void CMFCDlg::OnBnClickedOpen4()
{
	setName(m_eBoxOpenedFPth4, m_openedFPth4);
}
#endif

#if(0)
void CMFCDlg::OnBnClickedStartStop1()
{
	int period = getPeriod(m_eBoxSendPeriod1);
	::std::vector<char> fileData;
	if (!period)
	{
		::std::cout << "Frequency isn't set.\n"
			<< "Can't start sending." << ::std::endl;
		return;
	}
	else
	{
		::std::cout << "Frequency is set to " << period << ::std::endl;
	}


	if (m_openedFPth1 == "")
	{
		::std::cout << "File to send isn't set.\n"
			<< "Can't start sending." << ::std::endl;
		return;
	}

	//read file to buffer
	if (readFile(m_openedFPth1, fileData) != 0)
	{
		return;
	}

	//show new state
	toggleState(m_eBoxSendState1, sendState1);

	auto work = [&, fileData, period]() mutable {
		m_ftdiHandler.openDevice(m_selectedDevice);
		::std::cout << "Start sending data to the device "
			<< m_selectedDevice << ::std::endl;
		while (sendState1.load())
		{
			m_ftdiHandler.sendData(fileData, m_selectedDevice);
			::std::this_thread::sleep_for(::std::chrono::milliseconds(period));
		}
		m_ftdiHandler.closeDevice(m_selectedDevice);
	};
#if(1)
	m_future1 = ::std::async(std::launch::async, work);
#else
	m_worker1 = ::std::move(::std::thread(work));
#endif
}

void CMFCDlg::OnBnClickedStartStop2()
{
	toggleState(m_eBoxSendState2, sendState2);
}

void CMFCDlg::OnBnClickedStartStop3()
{
	toggleState(m_eBoxSendState3, sendState3);
}

void CMFCDlg::OnBnClickedStartStop4()
{
	toggleState(m_eBoxSendState4, sendState4);
}
#endif

