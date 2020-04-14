#include "pch.h"
#include "Dialog.h"

BEGIN_MESSAGE_MAP(Dialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()

	ON_CBN_SELCHANGE(IDC_SCAN_COMBO, &Dialog::OnCBoxSelDev)
	ON_BN_CLICKED(ID_BT_STOP_READ, &Dialog::OnBnClickedStop)

	ON_COMMAND_RANGE(
		ID_BT_OPEN_1, ID_BT_OPEN_20,
		openDispatch)

	ON_CONTROL_RANGE(EN_KILLFOCUS,
		IDC_EBOX_PERIOD_1, IDC_EBOX_PERIOD_20,
		periodEboxDispatch)

	ON_COMMAND_RANGE(
		IDC_CHBOX_START_STOP_1, IDC_CHBOX_START_STOP_20,
		startStopDispatch)


END_MESSAGE_MAP()

void Dialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCAN_COMBO, m_cBoxDevices);

	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		DDX_Control(pDX, IDC_EBOX_FILE_1 + idx,
			(*m_oneLine_arr[idx]).m_eBoxOpenedFPth);
		DDX_Control(pDX, IDC_EBOX_PERIOD_1 + idx,
			(*m_oneLine_arr[idx]).m_eBoxSendPeriod);
		DDX_Control(pDX, IDC_CHBOX_START_STOP_1 + idx,
			(*m_oneLine_arr[idx]).m_chBoxStartStop);
	}
	DDX_Control(pDX, IDC_EBOX_MED_RX_RATE, m_eBoxMedRXrate);
	DDX_Control(pDX, IDC_EBOX_MED_TX_RATE, m_eBoxMedTXrate);
}

void Dialog::OnBnClickedStop()
{
	m_ftdiHandler_ref.stopLogging();
}

void Dialog::OnCBoxSelDev()
{
	CString sel_device;
	m_cBoxDevices.GetLBText(m_cBoxDevices.GetCurSel(), sel_device);
	m_ftdiHandler_ref.setSelDev( utf16ToUtf8(sel_device.GetString()));
	UpdateData(FALSE);
}

void Dialog::openDispatch(UINT nID)
{
	if ((nID >= ID_BT_OPEN_1)
		&&(nID <= ID_BT_OPEN_20))
	{
		uint8_t idx = nID - ID_BT_OPEN_1;
		::std::cout << "Open" << (idx + 1) << " is pressed" << ::std::endl;
		(*m_oneLine_arr[idx]).openHndlr();
		m_stateHolder.saveState();
	}
	else
	{
		::std::cerr << "Unknown event code : " << nID << ::std::endl;
	}
}

void Dialog::periodEboxDispatch(UINT nID)
{
	if ((nID >= IDC_EBOX_PERIOD_1)
		&&(nID <= IDC_EBOX_PERIOD_20))
	{
		uint8_t idx = nID - IDC_EBOX_PERIOD_1;
		::std::cout << "Period " << (idx + 1) << " is set" << ::std::endl;
		(*m_oneLine_arr[idx]).periodHndlr();
		m_stateHolder.saveState();
	}
}

void Dialog::startStopDispatch(UINT nID)
{
	if ((nID >= IDC_CHBOX_START_STOP_1)
		&&(nID <= IDC_CHBOX_START_STOP_20))
	{
		uint8_t idx = nID - IDC_CHBOX_START_STOP_1;
		::std::cout << "Start/stop" << (idx + 1) << " is pressed" << ::std::endl;
		( * m_oneLine_arr[idx]).startStopHndlr();
	}
	else
	{
		::std::cerr << "Unknown event code : " << nID << ::std::endl;
	}
}

void Dialog::ftdiCallBack(const ::FTDI::FtdiHandler::EventCode& event,
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
		case ::FTDI::FtdiHandler::EventCode::NEW_DEV_SELECTED:
		{
			for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
			{
				(*m_oneLine_arr[idx]).stop();
			}
			break;
		}
		case ::FTDI::FtdiHandler::EventCode::MEDIUM_TX_RATE:
		{
			m_eBoxMedTXrate.SetWindowTextW(::std::get<1>(data));
			break;
		}
	}
}