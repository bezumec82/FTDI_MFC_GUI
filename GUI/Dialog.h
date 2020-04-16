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
#include "OneLine.h"
#include "StateHolder.h"

#define NUM_OF_SEND_LINES	20

// Dialog dialog
class Dialog : public CDialogEx
{
	friend class OneLine;
public: /*--- Aliases ---*/
	using OneLinePtr = ::std::unique_ptr<OneLine>;
public: /*--- Constructors ---*/
	Dialog(CWnd* pParent = nullptr); //standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCSIMPLE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); //DDX/DDV support

protected: /*--- Implementation ---*/
	HICON m_hIcon;

	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnClose();

	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public: /*--- Event handlers ---*/
	::FTDI::CallBack ftdiCallBack;

	//afx_msg void OnBnClickedScan();
	afx_msg void OnBnClickedStop();
	afx_msg void OnCBoxSelDev();

	/*--- Event dispatchers ---*/
	afx_msg void openDispatch(UINT nID);
	afx_msg void startStopDispatch(UINT nID);
	afx_msg void periodEboxDispatch(UINT nID);

private: /*--- Helpers ---*/
	OneLinePtr m_oneLine_arr[NUM_OF_SEND_LINES];

private: /*--- Control variables ---*/
	CComboBox m_cBoxDevices;

private: /*--- Utility variables ---*/
	::FTDI::FtdiHandler& m_ftdiHandler_ref;

private: /*--- Statistics ---*/
	CEdit m_eBoxMedRXrate;
	CEdit m_eBoxMedTXrate;
	CEdit m_eBoxImmTXrate;

private: /*--- Misc ---*/
	StateHolder m_stateHolder;
	::FTDI::EventBuffer m_eventBuffer;
};
