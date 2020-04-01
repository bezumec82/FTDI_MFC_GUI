#pragma once

#include "framework.h"
#include "Application.h"
#include "afxdialogex.h"

#include <sstream>
#include <codecvt>
#include <thread>


#include "ftdi.h"
#include "Utilities.h"
#include "OneLine.h"

// CMFCDlg dialog
class CMFCDlg : public CDialogEx
{
	friend class OneLine;
// Construction
public:
	CMFCDlg(CWnd* pParent = nullptr);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCSIMPLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected: /*--- Basic part ---*/
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public: /*--- Event handlers ---*/
#if (0)
	afx_msg void OnBnClickedOpen1();
	afx_msg void OnBnClickedOpen2();
	afx_msg void OnBnClickedOpen3();
	afx_msg void OnBnClickedOpen4();
	afx_msg void OnBnClickedStartStop1();
	afx_msg void OnBnClickedStartStop2();
	afx_msg void OnBnClickedStartStop3();
	afx_msg void OnBnClickedStartStop4();
#endif
	afx_msg void OnBnClickedScan();
	afx_msg void OnCbnSelchangeCombo();

	/*--- Event dispatchers ---*/
	afx_msg void openDispatch(UINT nID);
	afx_msg void startStopDispatch(UINT nID);

	afx_msg void OnBnClickedSave(); 
	afx_msg void OnBnClickedStartSave();

private: /*--- Helpers ---*/
	OneLine oneLine1;
	OneLine oneLine2;
	OneLine oneLine3;
	OneLine oneLine4;

private: /*--- Control variables ---*/
	CComboBox m_cBoxDevices;

private: /*--- Utility variables ---*/
	::FTDI::FtdiHandler m_ftdiHandler;

private: /*--- Read from device ---*/
	CEdit m_eBoxSaveFPth;
	CFile m_saveFile;
	CString m_saveFPth;
	::std::atomic_bool m_saveState{ false };
	::std::future<void> m_saveFuture;

private: /*--- Utitilities ---*/
	void setName(CEdit&, CString&);
	void toggleState(CEdit&, ::std::atomic_bool&);
	uint32_t getPeriod(CEdit&);
	int32_t readFile(CString& , ::std::vector<char>&);
};
