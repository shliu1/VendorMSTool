// VendorMSTool.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVendorMSToolApp:
// See VendorMSTool.cpp for the implementation of this class
//

class CVendorMSToolApp : public CWinApp
{
public:
	CVendorMSToolApp();

	void GetIniData();
	void SaveIniData();

	CString m_strInputFileName;
	BYTE  m_nSelectGetInformation;
	BYTE  m_nSelectSetControlData;
	DWORD m_dwSetControlData;
// 
	DWORD m_dwUSBConnect_1;
	DWORD m_dwUSBConnect_2;
	DWORD m_dwGetInformation_2;
	DWORD m_dwSetControlData_2;
    DWORD m_dwReadData_1;
	DWORD m_dwReadData_2;
	DWORD m_dwWriteData_1;
	DWORD m_dwWriteData_2;

    HINSTANCE  m_hWriteDll;
// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CVendorMSToolApp theApp;