#include "pch.h"
#include "Dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMFCDlg::CMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSIMPLE_DIALOG, pParent),
	oneLine1(m_ftdiHandler),
	oneLine2(m_ftdiHandler),
	oneLine3(m_ftdiHandler),
	oneLine4(m_ftdiHandler)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}




// CMFCDlg message handlers

BOOL CMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

#if(0)
	//Initialize captions
	m_eBoxSendState1.SetWindowTextW(L"stopped");
	m_eBoxSendState2.SetWindowTextW(L"stopped");
	m_eBoxSendState3.SetWindowTextW(L"stopped");
	m_eBoxSendState4.SetWindowTextW(L"stopped");

	m_eBoxOpenedFPth1.SetWindowTextW(L"<-select file");
	m_eBoxOpenedFPth2.SetWindowTextW(L"<-select file");
	m_eBoxOpenedFPth3.SetWindowTextW(L"<-select file");
	m_eBoxOpenedFPth4.SetWindowTextW(L"<-select file");
#endif
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCDlg::OnPaint()
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

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
