#pragma once

#include "framework.h"
#include "Application.h"
#include "afxdialogex.h"

#include <ctime>

#include <sstream>
#include <codecvt>
#include <thread>
#include <iomanip> // std::setprecision
#include <algorithm>
#include <any>
#include <functional>

#include "ftdi.h"
#include "Utilities.h"
#include "OneLine.h"
#include "TimeStat.h"

#define NUM_OF_SEND_LINES	20

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
	afx_msg void OnBnClickedScan();
	afx_msg void OnCbnSelchangeCombo();

	/*--- Event dispatchers ---*/
	afx_msg void openDispatch(UINT nID);
	afx_msg void startStopDispatch(UINT nID);

	afx_msg void OnBnClickedSave(); 
	afx_msg void OnChBoxStartStopSave();
	//afx_msg void OnCheckedImmSave();

private: /*--- Helpers ---*/
	::std::unique_ptr< OneLine > m_oneLine_uptr_arr[NUM_OF_SEND_LINES];

private: /*--- Control variables ---*/
	CComboBox m_cBoxDevices;

private: /*--- Utility variables ---*/
	::FTDI::FtdiHandler m_ftdiHandler;
	
private: /*--- Read from device ---*/
	::FTDI::Logger m_ftdiLogger;
	void loggerCallBack(const ::FTDI::Logger::EventCode&,
		::FTDI::Logger::Data&);
	CEdit m_eBoxSaveFPth;
	CEdit m_eBoxImmRXrate;
	CEdit m_eBoxMedRXrate;
	CButton m_chBoxStartStopSave;	
};
