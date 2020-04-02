#include "pch.h"
#include "Dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


CMFCDlg::CMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MFCSIMPLE_DIALOG, pParent)
{
	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		m_oneLine_uptr_arr[idx] = ::std::make_unique< OneLine >(m_ftdiHandler);
	}
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

BOOL CMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//Initialize captions

	for (int idx = 0; idx < NUM_OF_SEND_LINES; idx++)
	{
		(* m_oneLine_uptr_arr[idx] ).m_eBoxOpenedFPth.SetWindowTextW(L"<-select file");
	}
	m_eBoxImmRXrate.SetWindowTextW(L"0.0");
	m_eBoxMedRXrate.SetWindowTextW(L"0.0");

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
