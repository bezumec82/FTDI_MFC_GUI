#pragma once

#include "afxdialogex.h"
#include <iostream>
#include <atomic>
#include <future>
#include <string>

//#include "Utilities.h"

#include "ftdi.h"

class OneLine
{
public:
	class View
	{
	public: /*--- Construction ---*/
		View(OneLine& parent)
			: m_parent_ref{parent}
		{

		}

	public: /*--- Getters/Setters ---*/
		const CString& getFile()
		{
			return m_parent_ref.m_ftdiWriter.getFileName();
		}
		const int32_t getPeriod()
		{
			return m_parent_ref.m_ftdiWriter.getPerios();
		}

		void setFile(CString file_path)
		{
			m_parent_ref.openFile(file_path);
		}

	private: /*--- Variables ---*/
		OneLine& m_parent_ref;
	};

public:
	OneLine(FTDI::FtdiHandler& ftdiHandler)
		: m_ftdiHandler_ref{ ftdiHandler },
		m_ftdiWriter{ ftdiHandler },
		m_view(*this)
	{
		m_ftdiWriter.registerCallBack(::std::bind(&OneLine::writerCallBack, this,
			::std::placeholders::_1, ::std::placeholders::_2));
	}

public: /*--- Methods ---*/
	void writerCallBack(const ::FTDI::Writer::EventCode& errCode,
		const ::FTDI::Writer::Data& data);

public: /*--- Getters/Setters ---*/
	View& view()
	{
		return m_view;
	}

private: /*--- Implementation ---*/
	void getPeriod();
	int32_t openFile(CString);

public: /*--- Event handlers ---*/
	void openHndlr();
	void startStopHndlr();
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
	View m_view;

};

