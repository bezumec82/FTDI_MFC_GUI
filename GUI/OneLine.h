#pragma once

#include "afxdialogex.h"
#include <iostream>
#include <atomic>
#include <future>
#include <string>

#include "ftdi.h"

class OneLine
{
public:
	class View
	{
	public: /*--- Construction ---*/
		View(OneLine& parent)
			: m_parent_ref{ parent }
		{ }

	public: /*--- Getters/Setters ---*/
		// Opened file
		const CString getFile()
		{
			return m_parent_ref.m_ftdiWriter.getFileName();
		}
		void setFile(const CString& file_path)
		{
			m_parent_ref.openFile(file_path);
		}

		// Send period
		CString getPeriod()
		{
			return m_parent_ref.readPeriod();
		}
		void setPeriod(const CString& period)
		{
			m_parent_ref.writePeriod(period);
		}
	private: /*--- Variables ---*/
		OneLine& m_parent_ref;
	}; //end class

public:
	OneLine(FTDI::FtdiHandler& ftdiHandler)
		: m_ftdiHandler_ref{ ftdiHandler },
		m_ftdiWriter{ ftdiHandler },
		m_view(*this)
	{
#if(1)
		m_ftdiWriter.registerCallBack(::std::bind(\
			&OneLine::writerCallBack, this,
			::std::placeholders::_1, ::std::placeholders::_2));
#else
		//register event receiver from FTDI
		m_ftdiHandler_ref.registerCallBack(::std::bind(\
			&::FTDI::EventBuffer::receiveEvent, &m_eventBuffer,
			::std::placeholders::_1, ::std::placeholders::_2));
		//register local event handler
		m_eventBuffer.registerCallBack(::std::bind(\
			&OneLine::writerCallBack, this,
			::std::placeholders::_1, ::std::placeholders::_2));
#endif

	}

public: /*--- Getters/Setters ---*/
	View& view()
	{
		return m_view;
	}

private: /*--- Implementation ---*/
	CString readPeriod();
	void writePeriod(const CString&);

	int32_t openFile(CString);
	::FTDI::CallBack writerCallBack;

public: /*--- Event handlers ---*/

	void openHndlr();
	void periodHndlr();
	void startStopHndlr();

	void stop()
	{
		m_ftdiWriter.stop();
	}

private: /*--- Logic variables ---*/
	::FTDI::FtdiHandler& m_ftdiHandler_ref;
	::FTDI::Writer m_ftdiWriter;
	View m_view;
	::FTDI::EventBuffer m_eventBuffer;

public: /*--- GUI variables --- */
	CEdit m_eBoxOpenedFPth;
	CEdit m_eBoxSendPeriod;
	CButton m_chBoxStartStop;
};

