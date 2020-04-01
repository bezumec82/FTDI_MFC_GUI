#pragma once

#include "afxdialogex.h"
#include <iostream>
#include <atomic>
#include <future>
#include <string>

#include "Utilities.h"

#include "ftdi.h"

class OneLine
{
public:
	OneLine(::FTDI::FtdiHandler& ftdiHandler)
		: m_ftdiHandler_ref{ ftdiHandler }
	{
		//m_eBoxSendState.SetWindowTextW(L"stopped");
		//m_eBoxOpenedFPth.SetWindowTextW(L"<-select file");
	}

	/*--- GUI ---*/
	CEdit m_eBoxOpenedFPth;
	CEdit m_eBoxSendState;
	CEdit m_eBoxSendPeriod;

private:
	::FTDI::FtdiHandler& m_ftdiHandler_ref;
	/*--- Storage ---*/
	CString				m_openedFPth{ "" };
	::std::atomic_bool	m_sendState{ false };
	int32_t				m_period{ -1 };

	/*--- Send data ---*/
	::std::vector<char> m_fileDataBuf;
	BOOL fileOpened{ false };

	/*--- Activity ---*/
	::std::future<void> m_future;
	::std::thread		m_worker;

public:
	void OpenHndlr();
	void StartStopHndlr();

private: /*--- Utility ---*/
	void getPeriod();
	int32_t readFile();
	void toggleStateBox();
};

