#include "pch.h"
#include "OneLine.h"

/*-----------------*/
/*--- Utilities ---*/
/*-----------------*/

void OneLine::getPeriod()
{

	CString period_cstr;
	m_eBoxSendPeriod.GetWindowText(period_cstr);
	m_ftdiWriter.setPeriod(period_cstr);
}

void OneLine::writerCallBack(const ::FTDI::Writer::EventCode& errCode,
	const ::FTDI::Writer::Data& data)
{
	switch (errCode)
	{
		case ::FTDI::Writer::EventCode::NO_PERIOD_ERR:
		{
			m_chBoxStartStop.SetCheck(BST_UNCHECKED);
			break;
		}
		case ::FTDI::Writer::EventCode::NO_DATA_ERR:
		{
			m_chBoxStartStop.SetCheck(BST_UNCHECKED);
			break;
		}
		case ::FTDI::Writer::EventCode::FTDI_OPEN_ERR:
		{
			m_chBoxStartStop.SetCheck(BST_UNCHECKED);
			break;
		}
		case ::FTDI::Writer::EventCode::STOPPED:
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
	if (m_ftdiWriter.readFile() == 0)
	{
		CFile file(file_path, CFile::modeRead);
		m_eBoxOpenedFPth.SetWindowTextW(file.GetFileName());
	}
	else
	{
		return -1;
	}
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

void OneLine::startStopHndlr()
{
	if (m_ftdiWriter.isWriting())
	{
		m_ftdiWriter.stop();
	}
	else
	{
		getPeriod();
		m_ftdiWriter.start();
	}
}