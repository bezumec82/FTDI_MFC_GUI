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
	OneLine(FTDI::FtdiHandler& ftdiHandler)
		: m_ftdiHandler_ref{ ftdiHandler },
		m_ftdiWriter{ ftdiHandler }
	{
		m_ftdiWriter.registerCallBack(::std::bind(&OneLine::writerCallBack, this,
			::std::placeholders::_1, ::std::placeholders::_2));
	}

public: /*--- Methods ---*/
	void writerCallBack(const ::FTDI::Writer::EventCode& errCode,
		::FTDI::Writer::Data& data);
	void getPeriod();

public: /*--- Event handlers ---*/
	void OpenHndlr();
	void StartStopHndlr();
	void abort()
	{
		m_ftdiWriter.stop();
	}

private: /*--- Variables ---*/
	::FTDI::FtdiHandler& m_ftdiHandler_ref;
	::FTDI::Writer m_ftdiWriter;
public: /*--- Variables --- */
	CEdit m_eBoxOpenedFPth;
	CEdit m_eBoxSendPeriod;
	CButton m_chBoxStartStop;
};

