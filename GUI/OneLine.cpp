#include "pch.h"
#include "OneLine.h"

/*-----------------*/
/*--- Utilities ---*/
/*-----------------*/

CString OneLine::readPeriod()
{

	CString period;
	m_eBoxSendPeriod.GetWindowText(period);
	return period;
}

void OneLine::writePeriod(const CString& period)
{
	m_eBoxSendPeriod.SetWindowTextW(period);
}

void OneLine::writerCallBack(const ::FTDI::EventCode& errCode,
	const ::FTDI::Data& data)
{
	switch (errCode)
	{
		case ::FTDI::EventCode::NEW_DEV_SELECTED: //from 'FtdiHandler'
		case ::FTDI::EventCode::NO_PERIOD_ERR: //from 'FtdiWriter'
		case ::FTDI::EventCode::WRITE_FOPEN_ERR: //from 'FtdiWriter'
		case ::FTDI::EventCode::WRITE_STOPPED: //from 'FtdiWriter'
		{
			m_chBoxStartStop.SetCheck(BST_UNCHECKED);
			break;
		}
	}
}

int32_t OneLine::openFile(CString file_path)
{
	if (file_path.IsEmpty())
	{
		::std::cerr << "File path to open isn't set" << ::std::endl;
		return -1;
	}
	m_ftdiWriter.setFileName(file_path);
	CFile file(file_path, CFile::modeRead | CFile::shareDenyNone );
	m_eBoxOpenedFPth.SetWindowTextW(file.GetFileName());
	return 0;
}

/*----------------*/
/*--- Handlers ---*/
/*----------------*/
void OneLine::openHndlr()
{
	CFileDialog dlgFile(TRUE);
	if (dlgFile.DoModal() == IDOK)
	{
		openFile(dlgFile.GetPathName());
	}
}

void OneLine::periodHndlr()
{
	m_ftdiWriter.setPeriod(readPeriod());
}

void OneLine::startStopHndlr()
{
	if (m_ftdiWriter.isSending())
	{
		m_ftdiWriter.stop();
	}
	else
	{
		m_ftdiWriter.setPeriod(readPeriod());
		m_ftdiWriter.start();
	}
}