
// MFCApplication1.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
#error "include 'pch.h' before including this file for PCH"
#endif

#include <iostream>

#include "resource.h"		// main symbols




// CMFCApp:
// See MFCApplication1.cpp for the implementation of this class
//

class CMFCApp : public CWinApp
{
public:
	CMFCApp();

	// Overrides
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

	// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CMFCApp theApp;
