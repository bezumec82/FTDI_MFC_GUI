#include "pch.h"
#include "Dialog.h"

void CMFCDlg::setName(CEdit& dst, CString& file_path)
{
	CFileDialog dlgFile(TRUE); //open file
	if (dlgFile.DoModal() == IDOK)
	{
		file_path = dlgFile.GetPathName();
		dst.SetWindowTextW(file_path);
	}
}

void CMFCDlg::toggleState(CEdit& dst, ::std::atomic_bool& state)
{
	state.store(!state.load());
	if (state.load())
	{
		dst.SetWindowTextW(L"sending");
	}
	else
	{
		dst.SetWindowTextW(L"stopped");
	}
}

uint32_t CMFCDlg::getPeriod(CEdit& src)
{
	int ret_val = 0;
	CString freqText;
	src.GetWindowText(freqText);
	try
	{
		ret_val = ::std::stoi(freqText.GetString());
	}
	catch (const ::std::invalid_argument& ia)
	{
		std::cout << ia.what() << '\n';
		::std::cout << "Invalid send frequency value" << ::std::endl;
	}
	return ret_val;
}

int32_t CMFCDlg::readFile(CString& file_path, ::std::vector<char>& buf)
{
	CFile file;
	if (!file.Open(file_path, CFile::modeRead))
	{
		::std::cout << "Can't open file "
			<< utf16ToUtf8(file_path.GetString()) << ::std::endl;
		return -1;

	}
	buf.resize(UINT(file.GetLength()));
	UINT bytesRead = file.Read(buf.data(), UINT(file.GetLength()));
	file.Close();
	return 0;
}
