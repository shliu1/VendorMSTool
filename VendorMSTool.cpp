// VendorMSTool.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "VendorMSTool.h"
#include "VendorMSToolDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVendorMSToolApp
TCHAR	g_szModulePath[_MAX_PATH];

BEGIN_MESSAGE_MAP(CVendorMSToolApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()


// CVendorMSToolApp construction

CVendorMSToolApp::CVendorMSToolApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	m_strInputFileName = "";
	m_nSelectGetInformation = 0;
	m_nSelectSetControlData = 0;
	m_dwSetControlData = 0;

	m_dwUSBConnect_1 = 0;
	m_dwUSBConnect_2 = 0;
	m_dwGetInformation_2 = 0;
	m_dwSetControlData_2 = 0;
    m_dwReadData_1 = 0;
	m_dwReadData_2 = 0;
	m_dwWriteData_1 = 0;
	m_dwWriteData_2 = 0;
}


// The one and only CVendorMSToolApp object

CVendorMSToolApp theApp;


// CVendorMSToolApp initialization

BOOL CVendorMSToolApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	GetIniData();

	CVendorMSToolDlg dlg;
	m_pMainWnd = &dlg;

	CString csDllFileName;
	TCHAR szStr[_MAX_PATH];
    CString csDir;


    ::GetModuleFileName(NULL, szStr, _MAX_PATH);
    CString csStr(szStr);
	csDir = csStr.Left(csStr.ReverseFind('\\'));
	csDir += _T("\\");

	strcpy(g_szModulePath,csDir);
	
	csDllFileName = csDir + _T("VendorMassStor.dll");

    if ( m_hWriteDll == NULL )
	{
    	m_hWriteDll = ::LoadLibrary(csDllFileName);
		if ( m_hWriteDll == NULL )
		{
		    dlg.MessageBox(_T("VendorMassStor.dll does not exits"), _T("Error"), MB_OK|MB_ICONERROR);
			return FALSE;
 		}
	}


    
	dlg.m_strInputFileName = m_strInputFileName;
	dlg.m_nSelectGetInformation = m_nSelectGetInformation;
	dlg.m_nSelectSetControlData = m_nSelectSetControlData;	
	dlg.m_dwSetControlData = m_dwSetControlData;

	dlg.m_dwUSBConnect_1 = m_dwUSBConnect_1;
	dlg.m_dwUSBConnect_2 = m_dwUSBConnect_2;
	dlg.m_dwGetInformation_2 = m_dwGetInformation_2;
	dlg.m_dwSetControlData_2 = m_dwSetControlData_2;
    dlg.m_dwReadData_1 = m_dwReadData_1;
	dlg.m_dwReadData_2 = m_dwReadData_2;
	dlg.m_dwWriteData_1 = m_dwWriteData_1;
	dlg.m_dwWriteData_2 = m_dwWriteData_2;

	dlg.m_hWriteDll = m_hWriteDll;

	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
		 m_strInputFileName = dlg.m_strInputFileName;
	     m_nSelectGetInformation = dlg.m_nSelectGetInformation;
		 m_nSelectSetControlData = dlg.m_nSelectSetControlData;
	     m_dwSetControlData = dlg.m_dwSetControlData;

	     m_dwUSBConnect_1 = dlg.m_dwUSBConnect_1;
	     m_dwUSBConnect_2 = dlg.m_dwUSBConnect_2;
	     m_dwGetInformation_2 = dlg.m_dwGetInformation_2;
	     m_dwSetControlData_2 = dlg.m_dwSetControlData_2;
         m_dwReadData_1 = dlg.m_dwReadData_1;
	     m_dwReadData_2 = dlg.m_dwReadData_2;
	     m_dwWriteData_1 = dlg.m_dwWriteData_1;
	     m_dwWriteData_2 = dlg.m_dwWriteData_2;

		 SaveIniData();
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.

    FreeLibrary(m_hWriteDll);
    m_hWriteDll = NULL;


	return FALSE;
}

void CVendorMSToolApp::GetIniData()
{
	TCHAR szCurPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szCurPath, MAX_PATH);
	CString strCurPath = szCurPath;
	strCurPath = strCurPath.Left(strCurPath.ReverseFind('\\') + 1);

	CString strIniPathFile = strCurPath + (CString)"VendorMSTool.ini";

	CStdioFile fileIni;

	if(!fileIni.Open( strIniPathFile, CFile::modeRead | CFile::typeText ))
		return;

	CString strLine;

	while (fileIni.ReadString(strLine))
	{
		if(strLine.Find(TEXT("[Upload_File_Name]")) != -1)
		{
			fileIni.ReadString(strLine);
			m_strInputFileName = strLine;
		}
		if(strLine.Find(TEXT("[Select_Get_Information]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_nSelectGetInformation = (BYTE) atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Select_Set_Control_Data]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_nSelectSetControlData = (BYTE) atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Set_Control_Data]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwSetControlData = atol((char *)lpLine);
		}
// New Value
		if(strLine.Find(TEXT("[Value_USB_Connect_1]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwUSBConnect_1 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_USB_Connect_2]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwUSBConnect_2 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Set_Control_Data_2]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwSetControlData_2 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Get_Information_2]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwGetInformation_2 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Read_Data_1]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwReadData_1 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Read_Data_2]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwReadData_2 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Write_Data_1]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwWriteData_1 = atol((char *)lpLine);
		}
		if(strLine.Find(TEXT("[Value_Write_Data_2]")) != -1)
		{
			fileIni.ReadString(strLine);
			LPTSTR lpLine = strLine.GetBuffer(10);
			m_dwWriteData_2 = atol((char *)lpLine);
		}
	}

	fileIni.Close();
}

void CVendorMSToolApp::SaveIniData()
{	
	TCHAR szCurPath[MAX_PATH] = {0};
	GetModuleFileName(NULL, szCurPath, MAX_PATH);
	CString strCurPath = szCurPath;
	strCurPath = strCurPath.Left(strCurPath.ReverseFind('\\') + 1);

	CString strIniPathFile = strCurPath + (CString)"VendorMSTool.ini";

	CStdioFile fileIni;
	CString strTemp;

	if(!fileIni.Open( strIniPathFile, CFile::modeCreate | CFile::modeWrite | CFile::typeText ))
		return;

	fileIni.WriteString(TEXT("[Upload_File_Name]\r\n"));
	fileIni.WriteString(m_strInputFileName + (CString)"\n\r\n");

	fileIni.WriteString(TEXT("[Select_Get_Information]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_nSelectGetInformation);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Select_Set_Control_Data]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_nSelectSetControlData);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Set_Control_Data]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwSetControlData);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_USB_Connect_1]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwUSBConnect_1);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_USB_Connect_2]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwUSBConnect_2);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Set_Control_Data_2]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwSetControlData_2);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Get_Information_2]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwGetInformation_2);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Read_Data_1]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwReadData_1);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Read_Data_2]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwReadData_2);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Write_Data_1]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwWriteData_1);
	fileIni.WriteString(strTemp + (CString)"\n");

	fileIni.WriteString(TEXT("[Value_Write_Data_2]\r\n"));
	strTemp.Format(TEXT("%ld\r\n"), m_dwWriteData_2);
	fileIni.WriteString(strTemp + (CString)"\n");


	fileIni.Close();
}
