// VendorMSToolDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VendorMSTool.h"
#include "VendorMSToolDlg.h"

#include "dbt.h"
//#include "usbiodef.h"
//#include <winioctl.h>
#include <setupapi.h>
#include <initguid.h>   
//#include <iostream.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern   TCHAR	g_szModulePath[_MAX_PATH];

DWORD g_dwUSBUBPAddr;
BOOL  g_bClockSkew;

DWORD g_dwDQSODS;
DWORD g_dwCKDQSDS;
DWORD g_dwAddress;
BOOL  g_bRunFA93;
DWORD g_dwMaker[800];
//char  g_szPIDName[5];  
DWORD g_dwCurMode;  // FA93, FA95, FA92...etc
// FA95/FA92 boot code bytes
DWORD g_dwBootCodeBytes;

// FA95/FA92 declare
DWORD  g_adwSetAddr[100];
DWORD  g_adwSetVal[100];
DWORD  g_dwNo;


#define General_puropse	0
#define	NAND_Dump_Tool	1
#define Sample			2
#define	SD_Dump_Tool	3

// CAboutDlg dialog used for App About
// Product struct

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CVendorMSToolDlg dialog




CVendorMSToolDlg::CVendorMSToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CVendorMSToolDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    m_hWriteDll = NULL;
	m_hUploadThread = NULL;
    m_hDownloadThread = NULL;

}

void CVendorMSToolDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_EDIT_SELECT_FILE, m_editInputFileName);

	DDX_Control(pDX, IDC_COMBO_SET_CONTROL_DATA, m_comboSetControlData);
	DDX_Control(pDX, IDC_EDIT_SET_CONTROL_DATA, m_editSetControlData);

	DDX_Control(pDX, IDC_COMBO_GET_INFORMATION, m_comboGetInformation);
	DDX_Control(pDX, IDC_EDIT_GET_INFORMATION, m_editGetInformation);

	DDX_Control(pDX, IDC_COMBO_GET_TARGET, m_comboGetTarget);
	DDX_Control(pDX, IDC_EDIT_SELECT_WRITEFILE, m_editOutputFileName);

	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_EDIT_SET_SIZE, m_editSetSize);

	DDX_Control(pDX, IDC_COMBO_SELECT_DEVICE, m_comboSelectDevice);

	// Add some Edit 
	DDX_Control(pDX, IDC_EDIT_USBCONNECT_P1, m_editUSBConnect_1);
	DDX_Control(pDX, IDC_EDIT_USBCONNECT_P2, m_editUSBConnect_2);	
	DDX_Control(pDX, IDC_EDIT_GET_INFORMATION_P2, m_editGetInformation_2);
	DDX_Control(pDX, IDC_EDIT_SET_CONTROL_DATA_P2, m_editSetControlData_2);
	DDX_Control(pDX, IDC_EDIT_READ_DATA_P1, m_editReadData_1);
	DDX_Control(pDX, IDC_EDIT_READ_DATA_P2, m_editReadData_2);
	DDX_Control(pDX, IDC_EDIT_WRITE_DATA_P1, m_editWriteData_1);
	DDX_Control(pDX, IDC_EDIT_WRITE_DATA_P2, m_editWriteData_2);

	CDialog::DoDataExchange(pDX);

}

BEGIN_MESSAGE_MAP(CVendorMSToolDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
//	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, OnBnClickedOk)

	ON_BN_CLICKED(IDC_BUTTON_SELECT_FILE, OnBnClickedButtonSelectFile)

	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, OnBnClickedButtonUpload)

	ON_BN_CLICKED(IDC_BUTTON_USB_CONNECT, OnBnClickedButtonUsbConnect)

	ON_CBN_SELCHANGE(IDC_COMBO_SET_CONTROL_DATA, OnCbnSelchangeComboSetControlData)
	ON_BN_CLICKED(IDC_BUTTON_SET_CONTROL_DATA, OnBnClickedButtonSetControlData)

	ON_BN_CLICKED(IDC_BUTTON_GET_INFORMATION, OnBnClickedButtonGetInformation)
	ON_CBN_SELCHANGE(IDC_COMBO_GET_INFORMATION, OnCbnSelchangeComboGetInformation)

	ON_CBN_SELCHANGE(IDC_COMBO_GET_TARGET, OnCbnSelchangeComboGetTarget)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_WRITEFILE, OnBnClickedButtonWriteSelectFile)

	ON_BN_CLICKED(IDC_BUTTON_WRITE, OnBnClickedButtonDownload)


	ON_CBN_SELCHANGE(IDC_COMBO_SELECT_DEVICE, OnCbnSelchangeComboSelectDevice)
	ON_BN_CLICKED(IDC_BUTTON_CLOSEALL, &CVendorMSToolDlg::OnBnClickedButtonCloseall)
END_MESSAGE_MAP()


// CVendorMSToolDlg message handlers

BOOL CVendorMSToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	for (int i=0; i<4; i++)
		m_bOpenUSBDevice[i] = FALSE;

	m_editInputFileName.SetWindowText(m_strInputFileName);

	m_progress.SetRange(0, 100);
	m_nSelectGetTarget = 0;
	InitComboForGetTarget();
    InitComboForSetControlData();
    InitComboForGetInformation();
	m_nSelectDevice=0;
	InitComboForSelectDevice();


	char buffer[30];
	memset(buffer, 0, 30);

	sprintf(buffer, "%X", m_dwSetControlData);
	m_editSetControlData.SetWindowText((LPTSTR)buffer);


// Initial all parameter

    sprintf(buffer, "%d", m_dwUSBConnect_1);
    m_editUSBConnect_1.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwUSBConnect_2);
    m_editUSBConnect_2.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwSetControlData_2);
    m_editSetControlData_2.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwGetInformation_2);
    m_editGetInformation_2.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwReadData_1);
    m_editReadData_1.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwReadData_2);
    m_editReadData_2.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwWriteData_1);
    m_editWriteData_1.SetWindowText((LPTSTR)buffer);

    sprintf(buffer, "%d", m_dwWriteData_2);
    m_editWriteData_2.SetWindowText((LPTSTR)buffer);

		   
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVendorMSToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVendorMSToolDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}


// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVendorMSToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CVendorMSToolDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	char buffer[30];

	m_editSetControlData.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwSetControlData = strtoul (buffer, NULL, 16);

	m_editUSBConnect_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwUSBConnect_1 = strtoul (buffer, NULL, 10);

	m_editUSBConnect_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwUSBConnect_2 = strtoul (buffer, NULL, 10);

	m_editSetControlData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwSetControlData_2 = strtoul (buffer, NULL, 10);

	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10);

	m_editReadData_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwReadData_1 = strtoul (buffer, NULL, 10);

	m_editReadData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwReadData_2 = strtoul (buffer, NULL, 10);

	m_editWriteData_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwWriteData_1 = strtoul (buffer, NULL, 10);

	m_editWriteData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwWriteData_2 = strtoul (buffer, NULL, 10);

	OnOK();
}

void CVendorMSToolDlg::OnBnClickedButtonSelectFile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, NULL, 
//					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
					OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, 
					(LPCTSTR)_TEXT("All Files (*.*)|*.*||"), 
					NULL);

    // prompt the user for an input file
    dlg.m_ofn.lpstrTitle = TEXT("Select an saved file...");
	CString strCurPath = m_strInputFileName;
	strCurPath = strCurPath.Left(strCurPath.ReverseFind('\\') + 1);
	dlg.m_ofn.lpstrInitialDir = strCurPath;

    if (IDCANCEL == (int) dlg.DoModal())
        return;

    UpdateData(TRUE);

    // set the internal filename
    m_strInputFileName = dlg.GetPathName();
	m_editInputFileName.SetWindowText(m_strInputFileName);
	UpdateData(FALSE);
}

BOOL CVendorMSToolDlg::OpenUSBDevice()
{
	char buffer[1024];
	char *strUpperName ;
	::Sleep(3000);


    PFN_VENDORMASSSTOR_DRVCHECKDEVICEEXISTED pCheckDeviceExisted = (PFN_VENDORMASSSTOR_DRVCHECKDEVICEEXISTED)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
					"DrvCheckDeviceExisted");

	m_nStatus = pCheckDeviceExisted(_T("VID_0416"),_T("PID_4567"));

	if ( m_nStatus == FALSE )
	{
		m_szErrMsg = TEXT("\nCheck USB Device to be failed");
	    ExecResult(FAIL, m_szErrMsg);
		return FALSE;
 	}

	m_editUSBConnect_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwUSBConnect_1 = strtoul (buffer, NULL, 10);

	m_editUSBConnect_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwUSBConnect_2 = strtoul (buffer, NULL, 10);

   PFN_VENDORMASSSTOR_DRVUSBCONNECT pOpenDevice = (PFN_VENDORMASSSTOR_DRVUSBCONNECT)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
					"DrvUSBConnect");

	m_nStatus = pOpenDevice(m_dwUSBConnect_1, m_dwUSBConnect_2);

	if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	{
		m_szErrMsg = TEXT("\nConnect USB Device to be failed");
	    ExecResult(FAIL, m_szErrMsg);
		return FALSE;
 	}
	
    m_bOpenUSBDevice[m_nSelectDevice] = TRUE;
	return TRUE;
}

BOOL CVendorMSToolDlg::CloseUSBDevice()
{

    if ( m_hWriteDll == NULL )
	{
		return FALSE;
 	}
    PFN_VENDORMASSSTOR_DRVCLOSE pCloseDevice = (PFN_VENDORMASSSTOR_DRVCLOSE)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
					"DrvClose");
	m_nStatus = pCloseDevice();

	if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	{
		m_szErrMsg = TEXT("\n --> Close USB Device to be failed");
	    ExecResult(FAIL, m_szErrMsg);
		return FALSE;
 	}
    m_bOpenUSBDevice[m_nSelectDevice] = FALSE; 	
 	return TRUE;
}

void CVendorMSToolDlg::ExecResult(DWORD dwResult, CString strProcess)
{
	if ( dwResult == PASS )
	{    // Pass
		MessageBox(strProcess, _T("Success"), MB_OK);
	}
	else if (dwResult == FAIL)
	{    // Fail
	    MessageBox(strProcess, _T("Error"), MB_OK|MB_ICONERROR);
	}
}


void CVendorMSToolDlg::OnBnClickedButtonUpload()
{
		// TODO: Add your control notification handler code here
	if ( m_hUploadThread != NULL )
		return;
 	DWORD dwThreadID = -1;
	m_hUploadThread = ::CreateThread(NULL, 0, UploadThread, this, 0, &dwThreadID);
}

DWORD __stdcall CVendorMSToolDlg::UploadThread(LPVOID lpParm)
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
  // Read data into one file for testing
	
	CVendorMSToolDlg *pdlgUpload = (CVendorMSToolDlg *) lpParm;
	pdlgUpload->UploadProcess();
	pdlgUpload->m_hUploadThread = NULL;
	return 0;

}
void CVendorMSToolDlg::UploadProcess()
{
  	HANDLE hReadFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize, dwByteWrite, dwAllSize, dwMemorySize, dwUsedSize;
	BYTE *pbyBuffer;
	void *ptr=NULL;
	char buffer[1024];
	BOOL bOK=TRUE;
	HCURSOR hCursor, hWaitCursor;
	DWORD dwUsedKB;

//	   if ( OpenUSBDevice() == FALSE )
//		return;
	hWaitCursor = LoadCursor(NULL, IDC_WAIT);
	hCursor = SetCursor(hWaitCursor);
	m_editInputFileName.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	hReadFile = CreateFile(buffer, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, NULL);
    if (hReadFile != INVALID_HANDLE_VALUE)
    {

        if ( m_bOpenUSBDevice[m_nSelectDevice] == FALSE )
		{
			m_szErrMsg = TEXT("Please run USB Connect to work");
			ExecResult(FAIL, m_szErrMsg);
//			goto over;
				SetCursor(hCursor);
			return;
		}

		if ( ( m_nSelectGetTarget == 0 ) || ( m_nSelectGetTarget == 2 ))// General purpose and Sample
		{
			m_editSetSize.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
			dwFileSize = strtoul (buffer, NULL, 10);
			dwFileSize <<= 10;  // KB
        }
		else if ( m_nSelectGetTarget == 1 )  //Nand dump
		{

	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10);

			PFN_VENDORMASSSTOR_DRVGETINFORMATION pGetInformation = (PFN_VENDORMASSSTOR_DRVGETINFORMATION)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvGetInformation");
			if ( pGetInformation(IMAGE_SIZE, &dwFileSize, m_dwGetInformation_2) != eVENDORMASSSTOR_SUCCESS)
			{
			m_szErrMsg = TEXT("\nCan not obtain the information if image size!");
			ExecResult(FAIL, m_szErrMsg);
//			goto over;
				SetCursor(hCursor);
//			return;
			}
		}
		else if ( m_nSelectGetTarget == 3 )  // SD dump
		{
	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10);
			PFN_VENDORMASSSTOR_DRVGETINFORMATION pGetInformation = (PFN_VENDORMASSSTOR_DRVGETINFORMATION)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvGetInformation");
			if ( pGetInformation(IMAGE_SIZE, &dwFileSize,m_dwGetInformation_2) != eVENDORMASSSTOR_SUCCESS)
			{
			m_szErrMsg = TEXT("\nCan not obtain the information if image size!");
			ExecResult(FAIL, m_szErrMsg);

				SetCursor(hCursor);
//				goto over;
			return;
			}
		}
		else
		{
			m_szErrMsg = TEXT("\nSet wrong Taregt!");
			ExecResult(FAIL, m_szErrMsg);
//			goto over;
			return;
		}
	
	}
	else
	{
		m_szErrMsg = TEXT("\nUpload file does not exists\n");
		ExecResult(FAIL, m_szErrMsg);

			SetCursor(hCursor);
//			goto over;
			return;
	}
	
	m_editReadData_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwReadData_1 = strtoul (buffer, NULL, 10);
	m_editReadData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwReadData_2 = strtoul (buffer, NULL, 10);

 
    PFN_VENDORMASSSTOR_DRVREADDATA pReadData = (PFN_VENDORMASSSTOR_DRVREADDATA)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvReadData");

//    if ( dwFileSize >= 0x100000 )
		dwMemorySize = 0x100000;
//	else
//		dwMemorySize = dwFileSize;
	
	dwAllSize = (dwMemorySize+511)/512*512;
//	ptr = _aligned_malloc(dwAllSize, 16);
//	pbyBuffer = (BYTE *)ptr;
	ptr = (BYTE *)malloc(dwAllSize + 63);
	pbyBuffer = (BYTE *)(((DWORD)ptr+63) & 0xFFFFFFC0);

    int pos=0;
   	m_progress.SetPos(pos);
	if ( m_nSelectGetTarget == 3 )  // SD dump
	{
	dwAllSize = dwFileSize;  // KB unit
	dwUsedSize = 0;
	dwUsedKB = dwMemorySize >> 10;
	while (dwFileSize > 0)
	{
       if ( dwFileSize < dwUsedKB)
	   {
		   dwMemorySize = dwFileSize << 10;
		   dwUsedKB = dwFileSize;
	   }
	   m_nStatus = pReadData(pbyBuffer, dwMemorySize, m_dwReadData_1, m_dwReadData_2);
       if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	   {
	    	m_szErrMsg = TEXT("\nFail to read data from device\n");
		    ExecResult(FAIL, m_szErrMsg);
            bOK = FALSE;
		    goto over;
	   }
       if ( WriteFile(hReadFile, pbyBuffer, dwMemorySize, &dwByteWrite,  NULL) == 0 )
	   {
	    	m_szErrMsg = TEXT("\nFail to write into file\n");
		    ExecResult(FAIL, m_szErrMsg);
            bOK = FALSE;
	   }
		dwFileSize -= dwUsedKB;
		dwUsedSize += dwUsedKB;
//		pos = (int)(((double)dwUsedSize*100)/dwAllSize);
		pos = (int)((double)dwUsedSize/dwAllSize*100);
		m_progress.SetPos(pos);
		UpdateWindow();
//        SetWindowTheme(m_progress, L" ", L" ");
    }
	}
	else   // Keep byte unit
	{
	dwAllSize = dwFileSize; 
	dwUsedSize = 0;
	while (dwFileSize > 0)
	{
       if ( dwFileSize < dwMemorySize)
		   dwMemorySize = dwFileSize;
	   m_nStatus = pReadData(pbyBuffer, dwMemorySize, m_dwReadData_1, m_dwReadData_2);
       if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	   {
	    	m_szErrMsg = TEXT("\nFail to read data from device\n");
		    ExecResult(FAIL, m_szErrMsg);
            bOK = FALSE;
		    goto over;
	   }
       if ( WriteFile(hReadFile, pbyBuffer, dwMemorySize, &dwByteWrite,  NULL) == 0 )
	   {
	    	m_szErrMsg = TEXT("\nFail to write into file\n");
		    ExecResult(FAIL, m_szErrMsg);
            bOK = FALSE;
	   }
		dwFileSize -= dwMemorySize;
		dwUsedSize += dwMemorySize;
//		pos = (int)(((double)dwUsedSize*100)/dwAllSize);
		pos = (int)((double)dwUsedSize/dwAllSize*100);
		m_progress.SetPos(pos);
		UpdateWindow();
//        SetWindowTheme(m_progress, L" ", L" ");
    }
	}
	m_progress.SetPos(100);

over:
//	CloseUSBDevice();
	if (ptr != NULL )
//	   _aligned_free(ptr);
       free(ptr);
    if (hReadFile != INVALID_HANDLE_VALUE)
		CloseHandle(hReadFile);
/*	if ( bOK == TRUE )
	{
		m_szErrMsg = TEXT("\nRead Data OK\n");
		ExecResult(PASS, m_szErrMsg);
	}
	*/
	SetCursor(hCursor);
	{
		m_szErrMsg = TEXT("\nRead Data over !\n");
		ExecResult(PASS, m_szErrMsg);
	}

//	return;
}


void CVendorMSToolDlg::OnBnClickedButtonUsbConnect()
{
	// TODO: Add your control notification handler code here
	char buffer[20];
	BOOL bStatus;
	HCURSOR hCursor, hWaitCursor;

	hWaitCursor = LoadCursor(NULL, IDC_WAIT);
	hCursor = SetCursor(hWaitCursor);

    bStatus =  OpenUSBDevice();
	SetCursor(hCursor);

    if ( bStatus == FALSE )
	{
	    GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Disconnected   ");
	}
	else
	{
	    GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Connected	  ");
   	    PFN_VENDORMASSSTOR_DRVGETINFORMATION pGetInformation = (PFN_VENDORMASSSTOR_DRVGETINFORMATION)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvGetInformation");
// Connect and set Windows
        switch ( m_nSelectGetTarget)
		{
		case 0:  // general purpose
           GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_SET_CONTROL_DATA)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_GET_INFORMATION)->EnableWindow(TRUE);
            GetDlgItem(IDC_EDIT_SET_SIZE)->EnableWindow(TRUE);
			break;
		case 2:  // Sample
               GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_SET_CONTROL_DATA)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_GET_INFORMATION)->EnableWindow(TRUE);
            GetDlgItem(IDC_EDIT_SET_SIZE)->EnableWindow(TRUE);

			break;

		case 1:  // Nand Dump 
             GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_SET_CONTROL_DATA)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_GET_INFORMATION)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_SET_SIZE)->EnableWindow(FALSE);

	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10); 	
	        m_nStatus = pGetInformation( 0, &m_dwNandID,m_dwGetInformation_2);
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
			  return;
			}

	        m_nStatus = pGetInformation( 1, &m_dwNandPageSize, m_dwGetInformation_2);
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
               return;;
			}

	        m_nStatus = pGetInformation( 3, &m_dwNandBlockNumber, m_dwGetInformation_2);
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
               return;
			}

	        m_nStatus = pGetInformation( 4, &m_dwNandPagePerBlock, m_dwGetInformation_2);
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
              return;
			}

	        m_nStatus = pGetInformation( 2, &m_dwNandTotalSize, m_dwGetInformation_2);
			m_dwNandTotalSize >>= 20;
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
              return;
			}

			break;
		case 3:  // SD Dump 
            GetDlgItem(IDC_BUTTON_UPLOAD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_WRITE)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_SET_CONTROL_DATA)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_GET_INFORMATION)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_SET_SIZE)->EnableWindow(FALSE);


	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10); 	
	        m_nStatus = pGetInformation( 2, &m_dwNandTotalSize, m_dwGetInformation_2);
			m_dwNandTotalSize >>= 10;  // unit is KB for SD,
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		       m_szErrMsg = TEXT("\nFail to Get Information\n");
		       ExecResult(FAIL, m_szErrMsg);
               return;
			}



			break;
		}

	}

//over1:
//	CloseUSBDevice();
}


void CVendorMSToolDlg::OnCbnSelchangeComboSetControlData()
{
	// TODO: Add your control notification handler code here
    DWORD dwIndex; 
    UpdateData(TRUE);

    dwIndex = m_comboSetControlData.GetCurSel();
    m_nSelectSetControlData = (BYTE)dwIndex;

}

void CVendorMSToolDlg::InitComboForSetControlData()
{
	DWORD i;
	TCHAR   SetControlDataKeyName[8][12] = {
		"Control_0",
		"Control_1",
		"Control_2",
		"Control_3",
		"Control_4",
		"Control_5",
		"Control_6",
		"Control_7"
	};

    for ( i =0; i < 8; i++)
	{
        m_comboSetControlData.AddString(SetControlDataKeyName[i]);
    }

	m_comboSetControlData.SetCurSel(m_nSelectSetControlData);

	UpdateData(FALSE);
	
}


void CVendorMSToolDlg::OnBnClickedButtonSetControlData()
{
	// TODO: Add your control notification handler code here
	char buffer[12];
	BOOL bOK=TRUE;

//    if ( OpenUSBDevice() == FALSE )
//		return;


        if ( m_bOpenUSBDevice[m_nSelectDevice] == FALSE )
		{
			m_szErrMsg = TEXT("Please run USB Connect to work");
			ExecResult(FAIL, m_szErrMsg);
			return;
		}
		
	m_editSetControlData.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwSetControlData = strtoul (buffer, NULL, 16);

	m_editSetControlData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwSetControlData_2 = strtoul (buffer, NULL, 10);


    PFN_VENDORMASSSTOR_DRVSETCONTROLDATA pSetControlData = (PFN_VENDORMASSSTOR_DRVSETCONTROLDATA)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvSetControlData");
	
	m_nStatus = pSetControlData(m_nSelectSetControlData, m_dwSetControlData, m_dwSetControlData_2);
    if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	{
		m_szErrMsg = TEXT("\nFail to Set Control Data\n");
		ExecResult(FAIL, m_szErrMsg);
        bOK = FALSE;
	}
  
//	CloseUSBDevice();
	if ( bOK == TRUE )
	{
		m_szErrMsg = TEXT("\nSet Control Data OK\n");
		ExecResult(PASS, m_szErrMsg);
	}

	return;

}


void CVendorMSToolDlg::OnBnClickedButtonGetInformation()
{
	// TODO: Add your control notification handler code here
	char buffer[12];
	BOOL bOK;

//   if ( OpenUSBDevice() == FALSE )
//		return;


        if ( m_bOpenUSBDevice[m_nSelectDevice] == FALSE )
		{
			m_szErrMsg = TEXT("Please run USB Connect to work");
			ExecResult(FAIL, m_szErrMsg);
			return;
		}

	m_editGetInformation_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwGetInformation_2 = strtoul (buffer, NULL, 10);

    PFN_VENDORMASSSTOR_DRVGETINFORMATION pGetInformation = (PFN_VENDORMASSSTOR_DRVGETINFORMATION)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvGetInformation");
	
	m_nStatus = pGetInformation(m_nSelectGetInformation, &m_dwGetInformation, m_dwGetInformation_2);
    if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
	{
		m_szErrMsg = TEXT("\nFail to Get Information\n");
		ExecResult(FAIL, m_szErrMsg);
        bOK = FALSE;
	}
  
    sprintf(buffer, "%X", m_dwGetInformation);
	m_editGetInformation.SetWindowText((LPTSTR)buffer);

//	CloseUSBDevice();
	if ( bOK == TRUE )
	{
		m_szErrMsg = TEXT("\nGet Information OK\n");
		ExecResult(PASS, m_szErrMsg);
	}

}

void CVendorMSToolDlg::OnCbnSelchangeComboGetInformation()
{
	// TODO: Add your control notification handler code here
    DWORD dwIndex; 
    UpdateData(TRUE);

    dwIndex = m_comboGetInformation.GetCurSel();
    m_nSelectGetInformation = (BYTE) dwIndex;
}

void CVendorMSToolDlg::InitComboForGetInformation()
{
	DWORD i;
	TCHAR   GetInformationKeyName[8][20] = {
		"Image Height",
		"Image Width",
		"Image Size",
		"Information 3",
		"Information 4",
		"Information 5",
		"Information 6",
		"Information 7",
	};

	TCHAR   GetInformationKeyName_0[8][20] = {
		"Info 0",
		"Info 1",
		"Info 2",
		"Info 3",
		"Info 4",
		"Info 5",
		"Info 6",
		"Info 7",
	};

	if ( m_nSelectGetTarget == 1 )
	{
		for ( i =0; i < 8; i++)
		{
			m_comboGetInformation.AddString(GetInformationKeyName[i]);
		}

		m_comboGetInformation.SetCurSel(m_nSelectGetInformation);

		UpdateData(FALSE);
	}
	else if ( m_nSelectGetTarget == 0 )
	{

		for ( i =0; i < 8; i++)
		{
			m_comboGetInformation.AddString(GetInformationKeyName_0[i]);
		}

		m_comboGetInformation.SetCurSel(m_nSelectGetInformation);

		UpdateData(FALSE);
	}
}


void CVendorMSToolDlg::OnDestroy( )
{
	int i;
//	for (int i=0; i<g_dwSNNo; i++)
	{
		m_nSelectDevice = 0;
		if ( m_bOpenUSBDevice[m_nSelectDevice] == TRUE )
			CloseUSBDevice();
	}
	CDialog::OnDestroy();
}





void CVendorMSToolDlg::OnCbnSelchangeComboGetTarget()
{
	// TODO: Add your control notification handler code here
    DWORD dwIndex; 
    UpdateData(TRUE);

    dwIndex = m_comboGetTarget.GetCurSel();
    m_nSelectGetTarget = (BYTE)dwIndex;
    GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Run USB Connect !");

}

void CVendorMSToolDlg::InitComboForGetTarget()
{
	DWORD i;
	TCHAR   GetTargetKeyName[4][20] = {
		"General puropse",
	"NAND Dump Tool",
		"Sample",
		"SD Dump Tool"
	};

    for ( i =0; i < 4; i++)
	{
        m_comboGetTarget.AddString(GetTargetKeyName[i]);
    }

	m_comboGetTarget.SetCurSel(m_nSelectGetTarget);

	UpdateData(FALSE);
	
}

void CVendorMSToolDlg::OnBnClickedButtonWriteSelectFile()
{
	// TODO: Add your control notification handler code here
	CFileDialog dlg(TRUE, NULL, NULL, 
//					OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, 
					OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, 
					(LPCTSTR)_TEXT("All Files (*.*)|*.*||"), 
					NULL);

    // prompt the user for an input file
    dlg.m_ofn.lpstrTitle = TEXT("Select a download file...");
	CString strCurPath = m_strInputFileName;
	strCurPath = strCurPath.Left(strCurPath.ReverseFind('\\') + 1);
	dlg.m_ofn.lpstrInitialDir = strCurPath;

    if (IDCANCEL == (int) dlg.DoModal())
        return;

    UpdateData(TRUE);

    // set the internal filename
    m_strOutputFileName = dlg.GetPathName();
	m_editOutputFileName.SetWindowText(m_strOutputFileName);
	UpdateData(FALSE);
}

void CVendorMSToolDlg::OnBnClickedButtonDownload()
{
		// TODO: Add your control notification handler code here
	if ( m_hDownloadThread != NULL )
		return;
 	DWORD dwThreadID = -1;
	m_hDownloadThread = ::CreateThread(NULL, 0, DownloadThread, this, 0, &dwThreadID);
}

DWORD __stdcall CVendorMSToolDlg::DownloadThread(LPVOID lpParm)
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
  // Read data into one file for testing
	
	CVendorMSToolDlg *pdlgDownload = (CVendorMSToolDlg *) lpParm;
	pdlgDownload->DownloadProcess();
	pdlgDownload->m_hDownloadThread = NULL;
	return 0;

}

void CVendorMSToolDlg::DownloadProcess()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
  // Read data into one file for testing
  	HANDLE hReadFile = INVALID_HANDLE_VALUE;
    DWORD dwFileSize, dwByteWrite, dwAllSize, dwMemorySize, dwUsedSize;
	BYTE *pbyBuffer;
	void *ptr=NULL;
	char buffer[1024];
	BOOL bOK=TRUE;
	HCURSOR hCursor, hWaitCursor;
	DWORD dwSizeHigh, dwPercent, no,dwextra;

//	if ( OpenUSBDevice() == FALSE )
//		return;
	hWaitCursor = LoadCursor(NULL, IDC_WAIT);
	hCursor = SetCursor(hWaitCursor);
	m_editOutputFileName.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	hReadFile = CreateFile(buffer, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, NULL);
    if (hReadFile != INVALID_HANDLE_VALUE)
    {
        if ( m_bOpenUSBDevice[m_nSelectDevice] == FALSE )
		{
			m_szErrMsg = TEXT("Please run USB Connect to work");
			ExecResult(FAIL, m_szErrMsg);

				SetCursor(hCursor);
			return;
		}
		
/*
	    PFN_VENDORMASSSTOR_DRVGETINFORMATION pGetInformation = (PFN_VENDORMASSSTOR_DRVGETINFORMATION)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvGetInformation");
		if ( pGetInformation(IMAGE_SIZE, &dwFileSize) != eVENDORMASSSTOR_SUCCESS)
		{
			m_szErrMsg = TEXT("\nCan not obtain the information if image size!");
			ExecResult(FAIL, m_szErrMsg);
				SetCursor(hCursor);
			return;
		}
*/	
	}
	else
	{
		m_szErrMsg = TEXT("\nUpload file does not exists\n");
		ExecResult(FAIL, m_szErrMsg);
		SetCursor(hCursor);
		return;
	}
	m_editWriteData_1.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwWriteData_1 = strtoul (buffer, NULL, 10);
	m_editWriteData_2.GetWindowText ((LPTSTR)buffer, sizeof (buffer) / sizeof (buffer[0]));
	m_dwWriteData_2 = strtoul (buffer, NULL, 10);
	
 
    PFN_VENDORMASSSTOR_DRVREADDATA pWriteData = (PFN_VENDORMASSSTOR_DRVREADDATA)::GetProcAddress(
			(HMODULE)m_hWriteDll, 
				"DrvWriteData");

	
	dwFileSize = GetFileSize(hReadFile, &dwSizeHigh);
	dwMemorySize = 0x100000;

	dwAllSize = (dwMemorySize+511)/512*512;
//	ptr = _aligned_malloc(dwAllSize, 16);
//	pbyBuffer = (BYTE *)ptr;
	ptr = (BYTE *)malloc(dwAllSize + 63);
	pbyBuffer = (BYTE *)(((DWORD)ptr+63) & 0xFFFFFFC0);

	int pos=0;
   	m_progress.SetPos(pos);
	if ( m_nSelectGetTarget == 3 )  // SD dump
	{
// Write 4096 MB
    if (dwSizeHigh != 0 )
	{
		dwPercent = dwSizeHigh+1;
		dwextra = 100/dwPercent;
	}
	else
	{
		dwPercent = 1;
		dwextra = 100;
	}
	no = 0;
	while ( dwSizeHigh != 0 )
	{
		for (int i=0; i<4096; i++)
		{
			ReadFile(hReadFile, pbyBuffer, dwMemorySize, &dwByteWrite,  NULL);
	        m_nStatus = pWriteData(pbyBuffer, dwMemorySize, m_dwWriteData_1, m_dwWriteData_2);
            if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
			{
		        m_szErrMsg = TEXT("\nFail to read data from device\n");
		        ExecResult(FAIL, m_szErrMsg);
                bOK = FALSE;
		        goto over;
			}
			pos = (int)((double)i/4096*dwextra+ dwextra*no);
			m_progress.SetPos(pos);
			UpdateWindow();
        } 
		dwSizeHigh--;
		no++;
	}	
	dwAllSize = dwFileSize; 
	dwUsedSize = 0;
	while (dwFileSize > 0)
	{
       if ( dwFileSize < dwMemorySize)
		   dwMemorySize = dwFileSize;
		if ( ReadFile(hReadFile, pbyBuffer, dwMemorySize, &dwByteWrite,  NULL) == 0 )
		{
			m_szErrMsg = TEXT("\nFail to write into file\n");
			ExecResult(FAIL, m_szErrMsg);
			bOK = FALSE;
		}
	    m_nStatus = pWriteData(pbyBuffer, dwMemorySize,m_dwWriteData_1, m_dwWriteData_2);
        if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
		{
		  m_szErrMsg = TEXT("\nFail to read data from device\n");
		  ExecResult(FAIL, m_szErrMsg);
          bOK = FALSE;
		  goto over;
		}
        dwFileSize -= dwMemorySize;
		dwUsedSize += dwMemorySize;
//		pos = (int)(((double)dwUsedSize*100)/dwAllSize);
		pos = (int)((double)dwUsedSize/dwAllSize*dwextra+dwextra*no);
		m_progress.SetPos(pos);
		UpdateWindow();
//        SetWindowTheme(m_progress, L" ", L" ");
    }
	}
	else
	{
	dwAllSize = dwFileSize; 
	dwUsedSize = 0;
	while (dwFileSize > 0)
	{
       if ( dwFileSize < dwMemorySize)
		   dwMemorySize = dwFileSize;
		if ( ReadFile(hReadFile, pbyBuffer, dwMemorySize, &dwByteWrite,  NULL) == 0 )
		{
			m_szErrMsg = TEXT("\nFail to write into file\n");
			ExecResult(FAIL, m_szErrMsg);
			bOK = FALSE;
		}
	    m_nStatus = pWriteData(pbyBuffer, dwMemorySize,m_dwWriteData_1, m_dwWriteData_2);
        if ( m_nStatus != eVENDORMASSSTOR_SUCCESS )
		{
		  m_szErrMsg = TEXT("\nFail to read data from device\n");
		  ExecResult(FAIL, m_szErrMsg);
          bOK = FALSE;
		  goto over;
		}
        dwFileSize -= dwMemorySize;
		dwUsedSize += dwMemorySize;
//		pos = (int)(((double)dwUsedSize*100)/dwAllSize);
		pos = (int)((double)dwUsedSize/dwAllSize*100);
		m_progress.SetPos(pos);
		UpdateWindow();
//        SetWindowTheme(m_progress, L" ", L" ");
    }
	}
	m_progress.SetPos(100);
over:
//	CloseUSBDevice();
	if (ptr != NULL )
//	   _aligned_free(ptr);
       free(ptr);
    if (hReadFile != INVALID_HANDLE_VALUE)
		CloseHandle(hReadFile);
/*	if ( bOK == TRUE )
	{
		m_szErrMsg = TEXT("\nRead Data OK\n");
		ExecResult(PASS, m_szErrMsg);
	}
	*/
	SetCursor(hCursor);
//    if ( bOK == TRUE )
	{
		m_szErrMsg = TEXT("\nWrite Data over !\n");
		ExecResult(PASS, m_szErrMsg);
	}
    
//	return;
}



void CVendorMSToolDlg::OnCbnSelchangeComboSelectDevice()
{
	// TODO: Add your control notification handler code here
// TODO: Add your control notification handler code here
    DWORD dwIndex; 
    UpdateData(TRUE);

    dwIndex = m_comboSelectDevice.GetCurSel();
	if ( m_nSelectDevice != dwIndex )
	{
		if ( m_bOpenUSBDevice[dwIndex] == FALSE )
			GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Disconnected   ");
		else
			GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Connected   ");
	}
    m_nSelectDevice = (BYTE)dwIndex;
}

void CVendorMSToolDlg::InitComboForSelectDevice()
{
	DWORD i;
	TCHAR   GetTargetKeyName[4][20] = {
		"Device_1",
	    "Device_2",
		"Device_3",
		"Device_4"
	};

    for ( i =0; i < 4; i++)
	{
        m_comboSelectDevice.AddString(GetTargetKeyName[i]);
    }

	m_comboSelectDevice.SetCurSel(m_nSelectDevice);

	UpdateData(FALSE);
	
}

//Get the file TurboWriter.ini 

void CVendorMSToolDlg::OnBnClickedButtonCloseall()
{
	// TODO: Add your control notification handler code here
	int i;
//	for (int i=0; i<g_dwSNNo; i++)
	{
		m_nSelectDevice = 0;
		if ( m_bOpenUSBDevice[m_nSelectDevice] == TRUE )
		{
			CloseUSBDevice();
			m_bOpenUSBDevice[m_nSelectDevice] = FALSE;
		}
	}
	GetDlgItem(IDC_STATIC_SHOW_STATUS)->SetWindowText("Disconnected   ");
	m_nSelectDevice = 0;
	m_comboSelectDevice.SetCurSel(m_nSelectDevice);
	UpdateData(FALSE);
}
