#include "pch.h"
#include "Dialog.h"

BEGIN_MESSAGE_MAP(CMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()

	//ON_BN_CLICKED(ID_BT_SCAN, &CMFCDlg::OnBnClickedScan)
	ON_CBN_SELCHANGE(IDC_SCAN_COMBO, &CMFCDlg::OnCbnSelchangeCombo)
	ON_BN_CLICKED(ID_BT_STOP_READ, &CMFCDlg::OnBnClickedStop)

	ON_COMMAND_RANGE(
		ID_BT_OPEN_1, ID_BT_OPEN_20,
		openDispatch)
	ON_COMMAND_RANGE(
		IDC_CHBOX_START_STOP_1, IDC_CHBOX_START_STOP_20,
		startStopDispatch)

	//ON_BN_CLICKED(ID_BT_SAVE, &CMFCDlg::OnBnClickedSave)
	//ON_BN_CLICKED(IDC_CHBOX_START_STOP_SAVE,
	//	&CMFCDlg::OnChBoxStartStopSave)
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

	//DDX_Control(pDX, IDC_EBOX_SAVE_FILE, m_eBoxSaveFPth);
	//DDX_Control(pDX, IDC_CHBOX_START_STOP_SAVE, m_chBoxStartStopSave);
	//DDX_Control(pDX, IDC_EBOX_IMM_RX_RATE, m_eBoxImmRXrate);

	DDX_Control(pDX, IDC_EBOX_MED_RX_RATE, m_eBoxMedRXrate);
	DDX_Control(pDX, IDC_EBOX_MED_TX_RATE, m_eBoxMedTXrate);
}

#if(0)
void CMFCDlg::OnBnClickedScan()
{
	if (!m_ftdiHandler.findFtdiDevices())
		m_ftdiHandler.printFtdiDevices();
}
#endif

void CMFCDlg::OnBnClickedStop()
{

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
		(*m_oneLine_uptr_arr[idx]).openHndlr();
		m_stateHolder.saveState();
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
		( * m_oneLine_uptr_arr[idx]).startStopHndlr();
	}
	else
	{
		::std::cerr << "Unknown event code : " << nID << ::std::endl;
	}
}

#if(0)
void CMFCDlg::OnBnClickedSave()
{
	CFileDialog dlgFile(FALSE); //save file
	if (dlgFile.DoModal() == IDOK)
	{
		CString file_path = dlgFile.GetPathName();
		m_ftdiLogger.setFileName(file_path);

		CFile file(file_path, CFile::modeRead);
		m_eBoxSaveFPth.SetWindowTextW(file.GetFileName());
	}
}


void CMFCDlg::loggerCallBack(const ::FTDI::Logger::EventCode& errCode,
	const ::FTDI::Logger::Data& data)
{
	switch (errCode)
	{
		case ::FTDI::Logger::EventCode::NO_FILE_NAME_ERR:
		{
			m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
			break;
		}
		case ::FTDI::Logger::EventCode::FOPEN_ERR:
		{
			m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
			break;
		}
		case ::FTDI::Logger::EventCode::STOPPED:
		{
			m_chBoxStartStopSave.SetCheck(BST_UNCHECKED);
			m_eBoxImmRXrate.SetWindowTextW(L"0.0");
			break;
		}
		case ::FTDI::Logger::EventCode::IMMEDIATE_RX_RATE:
		{
			m_eBoxImmRXrate.SetWindowTextW(::std::get<CString>(data));
			break;
		}
		case ::FTDI::Logger::EventCode::MEDIUM_RX_RATE:
		{
			m_eBoxMedRXrate.SetWindowTextW(::std::get<CString>(data));
			break;
		}
	}//end switch
}

void CMFCDlg::OnChBoxStartStopSave()
{
	if (m_ftdiLogger.isLogging()) //reading/saving in process
	{
		m_ftdiLogger.stop();
	}
	else m_ftdiLogger.start();
}
#endif
void CMFCDlg::ftdiCallBack(const ::FTDI::FtdiHandler::EventCode& event,
	const ::FTDI::FtdiHandler::Data& data)
{
	switch (event)
	{
	case ::FTDI::FtdiHandler::EventCode::SCAN_DATA:
	{
		//re-fill combo box
		m_cBoxDevices.ResetContent();
		::FTDI::DevDescriptions dev_descs = ::std::get<::FTDI::DevDescriptions>(data);
		for (const ::std::string& desc : dev_descs)
		{
			m_cBoxDevices.AddString(CString{ desc.c_str() });
		}
		break;
	}

	}
}