// VendorMSToolDlg.h : header file
//
#include "VendorMassStor_Interface.h"
//#include "MacProgressCtrl.h"

//#define COLOR_DOWNLOAD   RGB(90,200,60)
//#define COLOR_PARSE      RGB(17, 196, 62)
//#include "TextProgressCtrl.h"
#include <vector>
#include <string>
#include <iostream>

//#include "ntddscsi.h"
//#include "ntddstor.h"
//#include "Spti.h"
//#include "TextProgressCtrl.h"

// From FA93, it is a new style recovery mode
#define MODE_FA91			0
#define MODE_FA93			1
#define MODE_FA95			2
#define MODE_FA92			3

//#define NAND_DEBUG
#define RECOVERY_MODE		0x01
#define NORMAL_MODE			0x02
#define UNKNOWN_MODE		0x00


#define MODE_SDRAM            0x00
#define MODE_NOR              0x01
#define MODE_NOR_RAW_DATA     0x02
#define MODE_NAND             0x03
#define MODE_NAND_RAW_DATA    0x04
#define MODE_SPI              0x05
#define MODE_SPI_RAW_DATA     0x06
#define MODE_NOR_IMAGE_LIST   0x07
#define MODE_NAND_INAGE_LIST  0x08
#define MODE_SPI_IMAGE_LIST   0x09
#define MODE_NOR_FLASH_TYPE   0x0A

#define I_STRUCTURE      0x00
#define I_DATA           0x01
#define MAX_DRIVER	32

//FOR VASERIES
#define MODE_SDCARD             0x07
////Get Image List ////////////////////////////////////////////
//#define NOR_IMAGELIST         0x00
#define NAND_nIMAGELIST         0x01
//#define SPI_IMAGELIST         0x02
#define SDCARD_nIMAGELIST       0x03
//#define NOR_FLASHTYPE         0x04
#define SPI_nIMAGELIST          0x02

//Get NAND Block Information 
#define GET_COUNT             0x0
#define GET_DATA              0x1
#define GET_SPI_READY         0x2
#define GET_LAST_STATUS       0x3
#define GET_NAND_SIZE         0x4 //by MBytes
#define GET_SDCARD_SIZE       0x5 //by MBytes
#define GET_NAND_FLASH_ID	  0x6 // Get flash ID, return 5 bytes, otherwise 4 bytes
#define GET_SPI_DISK_SIZE	  0x7 // Get SPI flash size	

#define SEND_RESET              0x2


//#define IBR_MASSSTOR          "B190"
//#define VASERIES_MASSSTOR_0   "B191"
//#define VASERIES_MASSSTOR_1   "B192"
//#define VASERIES_MASSSTOR_1   "100E"

//#define VASERIES_MASSSTOR_1   "B190"
#define IBR_MASSSTOR_FA93		"9396"
#define VASERIES_MASSSTOR_FA93	"4567"

#define IBR_MASSSTOR_FA95		"9596"
#define VASERIES_MASSSTOR_FA95	"4567"  //"5595"

#define IBR_MASSSTOR_FA92		"9296"
#define VASERIES_MASSSTOR_FA92	"4567"

//VENDOR ID 

#define SCSIOP_VENDOR_SendInformation      0x61  
#define SCSIOP_VENDOR_GetImageList         0x62
#define SCSIOP_VENDOR_Write			       0x63
#define SCSIOP_VENDOR_Verify			   0x64
#define SCSIOP_VENDOR_GetInformation       0x65
#define SCSIOP_VENDOR_SetInformation       0x66


//Set Information
#define SET_NAND_RESERVEDSIZE    0x0     //Set NAND system reserved area size by MBytes
#define SET_SDCARD_RESERVEDSIZE  0x1
#define SET_NAND_PAGES			 0x2    // Set NAND pages per block.
#define SET_NAND_BLOCKS			 0x3    // Set NAND blocks per flash
#define SET_SD_FORMAT			 0x4    // Set SD Format,
#define SET_SD_PORT				 0x5    // Set SD Port

#define NAND_SYSTEM    0
#define NAND_USER_1    1
#define NAND_USER_2    2
#define SD_SYSTEM      3
#define SD_USER        4


#define		PASS			1
#define		FAIL			0
#define     ACCESSNO		8*1024

//The input parameter for Set Control Data 0x61
#define		CONTROL_0		0
#define		CONTROL_1		1
#define		CONTROL_2		2
#define		CONTROL_3		3
#define		CONTROL_4		4
#define		CONTROL_5		5


//The input parameter for Get Information -- 0x63
#define		IMAGE_HEIGHT	0
#define		IMAGE_WIDTH		1
#define		IMAGE_SIZE		2
#define		INFORMATION_0	3
#define		INFORMATION_1	4
#define		INFORMATION_2	5

typedef struct _XBINHEAD
{
	DWORD sign;
	DWORD address;
	DWORD length;
	DWORD version;

}XBINHEAD,*PXBINHEAD;


#pragma once


// CVendorMSToolDlg dialog
class CVendorMSToolDlg : public CDialog
{
// Construction
public:
	CVendorMSToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VENDORMSTOOL_DIALOG };

public:
	CEdit	m_editInputFileName;
	CEdit	m_editSetControlData;
	CEdit   m_editGetInformation;
	CEdit	m_editOutputFileName;
	CEdit	m_editSetSize;


	CEdit   m_editUSBConnect_1;
	CEdit   m_editUSBConnect_2;
	CEdit   m_editGetInformation_2;
	CEdit   m_editSetControlData_2;
	CEdit   m_editReadData_1;
	CEdit   m_editReadData_2;
    CEdit   m_editWriteData_1;
	CEdit   m_editWriteData_2;


	CComboBox	m_comboSetControlData;
	CComboBox	m_comboGetInformation;
	CComboBox	m_comboGetTarget;
	CComboBox	m_comboSelectDevice;

//	CTextProgressCtrl	m_progress;
	CProgressCtrl	m_progress;
	HANDLE          m_hUploadThread;
	HANDLE          m_hDownloadThread;

	CString m_strInputFileName;
	CString m_strOutputFileName;
	BYTE   m_nSelectGetInformation;
	BYTE   m_nSelectSetControlData;
	BYTE   m_nSelectGetTarget;
	BYTE   m_nSelectDevice;

	DWORD	m_dwSetControlData;
	DWORD   m_dwGetInformation;

	DWORD  m_dwUSBConnect_1;
	DWORD  m_dwUSBConnect_2;
	DWORD  m_dwGetInformation_2;
	DWORD  m_dwSetControlData_2;
    DWORD  m_dwReadData_1;
	DWORD  m_dwReadData_2;
	DWORD  m_dwWriteData_1;
	DWORD  m_dwWriteData_2;

	DWORD	m_dwCurIndex;

	int		   m_nStatus;
	CString	   m_szErrMsg;
    HINSTANCE  m_hWriteDll;
// USB device Open?
	BOOL	m_bOpenUSBDevice[4];

// Ray added
	HANDLE	m_handle[26];//
	HANDLE  Old_handle;
	CString	m_pid;
	CString	m_vid;
	CString m_usbid;
	CString m_sDest[8];//
	CString exePath;
	long m_length;		


    int  m_iTarget;   //0 ~ 2
	DWORD m_dwNandID;
	DWORD m_dwNandPageSize;
	DWORD m_dwNandBlockNumber;
	DWORD m_dwNandTotalSize;
	DWORD m_dwNandPagePerBlock;

	char g_strSN[4][256];
protected:
	BOOL OpenUSBDevice();
	BOOL CloseUSBDevice();
    void ExecResult(DWORD dwResult, CString strProcess);

	void InitComboForSetControlData();
	void InitComboForGetInformation();
	void InitComboForGetTarget();
	void InitComboForSelectDevice();
		
    static DWORD __stdcall UploadThread(LPVOID lpParm);
    void UploadProcess();

    static DWORD __stdcall DownloadThread(LPVOID lpParm);
    void DownloadProcess();



	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
public:
	afx_msg void OnBnClickedButtonSelectFile();


public:
	afx_msg void OnBnClickedButtonUpload();
public:
	afx_msg void OnBnClickedButtonUsbConnect();

	afx_msg void OnCbnSelchangeComboSetControlData();
	afx_msg void OnBnClickedButtonSetControlData();
	afx_msg void OnBnClickedButtonSetControlData2();
	afx_msg void OnBnClickedButtonGetInformation();
	afx_msg void OnCbnSelchangeComboGetInformation();
// Ray
    afx_msg void OnCbnSelchangeComboGetTarget();
	afx_msg void OnBnClickedButtonWriteSelectFile();
	afx_msg void OnBnClickedButtonDownload();
    afx_msg void OnDestroy( );

	afx_msg void OnCbnSelchangeComboSelectDevice();
	afx_msg void OnBnClickedButtonCloseall();
};
