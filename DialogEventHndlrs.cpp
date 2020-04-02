#include "pch.h"
#include "Dialog.h"

BEGIN_MESSAGE_MAP(CMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()

	ON_BN_CLICKED(ID_BT_SCAN, &CMFCDlg::OnBnClickedScan)
	ON_CBN_SELCHANGE(IDC_SCAN_COMBO, &CMFCDlg::OnCbnSelchangeCombo)

	ON_COMMAND_RANGE(
		ID_BT_OPEN_1, ID_BT_OPEN_20, 
		openDispatch)
	ON_COMMAND_RANGE(
		IDC_CHBOX_START_STOP_1, IDC_CHBOX_START_STOP_20, 
		startStopDispatch)

	ON_BN_CLICKED(ID_BT_SAVE, &CMFCDlg::OnBnClickedSave)
	ON_BN_CLICKED(IDC_CHBOX_START_STOP_SAVE, 
		&CMFCDlg::OnChBoxStartStopSave)
	//ON_BN_CLICKED(IDC_CHECK_IMM_SAVE, &CMFCDlg::OnCheckedImmSave)
END_MESSAGE_MAP()

void CMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCAN_COMBO, m_cBoxDevices);

	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		DDX_Control(pDX, IDC_EBOX_FILE_1 + idx, 
			(*m_oneLine_uptr_arr[idx]).m_eBoxOpenedFPth);
		DDX_Control(pDX, IDC_EBOX_PERIOD_1 + idx, 
			(*m_oneLine_uptr_arr[idx]).m_eBoxSendPeriod);
		DDX_Control(pDX, IDC_CHBOX_START_STOP_1 + idx, 
			(*m_oneLine_uptr_arr[idx]).m_chBoxStartStop);
	}

	DDX_Control(pDX, IDC_EBOX_SAVE_FILE, m_eBoxSaveFPth);
	DDX_Control(pDX, IDC_CHBOX_START_STOP_SAVE, m_chBoxStartStopSave);
	DDX_Control(pDX, IDC_EBOX_IMM_RX_RATE, m_eBoxImmRXrate);
	DDX_Control(pDX, IDC_EBOX_MED_RX_RATE, m_eBoxMedRXrate);
}

void CMFCDlg::OnBnClickedScan()
{
	if (!m_ftdiHandler.findFTDIDevices())
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
	if (nID <= ID_BT_OPEN_20)
	{
		uint8_t idx = nID - ID_BT_OPEN_1;
		::std::cout << "Open" << (idx + 1) << " is pressed" << ::std::endl;
		(*m_oneLine_uptr_arr[idx]).OpenHndlr();
	}
	else
	{
		::std::cerr << "Unknown event code : " << nID << ::std::endl;
	}
}

void CMFCDlg::startStopDispatch(UINT nID)
{
	if (nID <= IDC_CHBOX_START_STOP_20)
	{
		uint8_t idx = nID - IDC_CHBOX_START_STOP_1;
		::std::cout << "Start/stop" << (idx + 1) << " is pressed" << ::std::endl;
		( * m_oneLine_uptr_arr[idx]).StartStopHndlr();
	}
	else
	{
		::std::cerr << "Unknown event code : " << nID << ::std::endl;
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

//void CMFCDlg::OnCheckedImmSave()
//{
//	m_saveImmediately.store(!m_saveImmediately.load());
//}

#define SAVE_PERIOD_MS 100
void CMFCDlg::OnChBoxStartStopSave()
{
	CFileException ex;

	//handle stop
	if (m_saveState.load()) //reading/saving in process
	{
		m_saveState.store(false);
		m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
		return;
	}

	//open file
	if ( m_saveFPth.IsEmpty() )
	{
		::std::cerr << "File to save is not set" << ::std::endl;
		m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
		return;
	}
	if (!m_saveFile.Open(m_saveFPth,
		CFile::modeWrite 
		| CFile::modeCreate 
		| CFile::shareDenyWrite, &ex))
	{
		TCHAR szCause[255] = { 0 };
		ex.GetErrorMessage(szCause, sizeof(szCause)/2 - 1);
		::std::cout << "Can't create/open file "
			<< utf16ToUtf8(m_saveFPth.GetString()) << '\n'
			<< "Error : " << utf16ToUtf8(szCause)
			<< ::std::endl;
		m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
		return;
	}

	//reading to the file
	auto work = [&]() mutable
	{
		TimeStat time_stat;
		//try to open device
		if (m_ftdiHandler.openDevice() == 0)
		{
			::std::cout << "Start reading data from the device "
						<< m_ftdiHandler.getSelDev() << ::std::endl;
			m_saveState.store(true);
			m_ftdiHandler.clearRxBuf();
			time_stat.start();
		}
		else
		{
			m_ftdiHandler.closeDevice(); //try to fix situation
		}
		while (m_saveState.load())
		{
			::std::vector<char> buffer;
			if( m_ftdiHandler.recvData(buffer) != 0)
				break;
			m_saveFile.Write(buffer.data(), buffer.size());
			//if(m_saveImmediately.load())
				m_saveFile.Flush();

			m_eBoxImmRXrate.SetWindowTextW( \
				time_stat.getImmRXrate(buffer.size()));
			m_eBoxMedRXrate.SetWindowTextW( \
				time_stat.getMedRXrate(buffer.size()));

			::std::this_thread::sleep_for( \
				::std::chrono::milliseconds(SAVE_PERIOD_MS));
		}
		m_eBoxImmRXrate.SetWindowTextW(L"0.0");
		m_ftdiHandler.closeDevice();

		time_stat.stop();

		m_saveFile.Flush();
		m_saveFile.Close();
		m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
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

