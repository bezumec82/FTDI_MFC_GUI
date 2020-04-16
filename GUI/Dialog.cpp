#include "pch.h"
#include "Dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Dialog::Dialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSIMPLE_DIALOG, pParent),
	m_ftdiHandler_ref {FTDI::FtdiHandler::getInstance()}
{
	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		m_oneLine_arr[idx] = \
			::std::make_unique< OneLine >(m_ftdiHandler_ref);
		m_stateHolder.registerWriter(idx, m_oneLine_arr[idx]->view());
	}
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL Dialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_eventBuffer.start();
	//register event receiver from FTDI
	m_ftdiHandler_ref.registerCallBack(::std::bind(\
		& ::FTDI::EventBuffer::receiveEvent, &m_eventBuffer,
		::std::placeholders::_1, ::std::placeholders::_2));
	//register local event handler
	m_eventBuffer.registerCallBack(::std::bind(\
		& Dialog::ftdiCallBack, this,
		::std::placeholders::_1, ::std::placeholders::_2));

	//Initialize captions
	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		( * m_oneLine_arr[idx] ).m_eBoxOpenedFPth.SetWindowTextW(L"<-select file");
		( * m_oneLine_arr[idx] ).m_eBoxSendPeriod.SetWindowTextW(L"0");
	}
	//autoscan
	m_stateHolder.restoreState();
	m_ftdiHandler_ref.startScan();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void Dialog::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

void Dialog::OnClose()
{
	::std::cout << "Stop all activity" << ::std::endl;
	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		(*m_oneLine_arr[idx]).stop();
	}
	m_ftdiHandler_ref.abort();
	m_eventBuffer.stop();
	::std::this_thread::sleep_for(::std::chrono::milliseconds(300));
	CDialogEx::OnClose();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR Dialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
