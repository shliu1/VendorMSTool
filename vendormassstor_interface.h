//----------------------------------------------------------------------------
// 
// Copyright (c) Nuvoton Technology Corp. All rights reserved.
//
//----------------------------------------------------------------------------


#ifndef __VENDORMASSSTOR_INTERFACE_H__
#define __VENDORMASSSTOR_INTERFACE_H__


#ifdef COMMU_DLL
    #define COMMU_API	__declspec(dllexport)
 
#else
    #define COMMU_API	__declspec(dllimport)
#endif


#include <map>
using namespace std;

// Active following definitions to avoid some types redefinition
#define __NVTTYPE_CHAR_DEFINED__
#define __NVTTYPE_PCHAR_DEFINED__
#define __NVTTYPE_PSTR_DEFINED__
#define __NVTTYPE_PCSTR_DEFINED__
#define __NVTTYPE_SIZE_T_DEFINED__
#include "NVTTypes.h"


typedef INT32	ERRCODE;

enum {
	eVENDORMASSSTOR_SUCCESS				= 0,	// Successful with no error.
	eVENDORMASSSTOR_ERR_NULL_POINTER    = 1,    // buffer address is null
	eVENDORMASSSTOR_ERR_BAD_SIZE		= 2,	// lenght is 0
	eVENDORMASSSTOR_FAILURE				= 0xFFFFFFFF,	// failure
}; // enum



//----------------------------------------------------------------------------
// API function pointer type
//----------------------------------------------------------------------------
typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVCHECKDEVICEEXISTED_X)(char *, char *, char *, DWORD);
typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVUSBCONNECT_X)(DWORD, DWORD, DWORD);
typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVCLOSE_X)(DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVSETCONTROLDATA_X)(DWORD, DWORD, DWORD, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVGETINFORMATION_X)(DWORD , DWORD *, DWORD, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVREADDATA_X)(BYTE *, DWORD, DWORD, DWORD, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVWRITEDATA_X)(BYTE *, DWORD, DWORD, DWORD, DWORD);

typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVCHECKDEVICEEXISTED)(char *, char *);
typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVUSBCONNECT)(DWORD, DWORD);
typedef ERRCODE (*PFN_VENDORMASSSTOR_DRVCLOSE)();
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVSETCONTROLDATA)(DWORD, DWORD, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVGETINFORMATION)(DWORD , DWORD *, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVREADDATA)(BYTE *, DWORD, DWORD, DWORD);
typedef ERRCODE	(*PFN_VENDORMASSSTOR_DRVWRITEDATA)(BYTE *, DWORD, DWORD, DWORD);

//----------------------------------------------------------------------------
// API prototype
//----------------------------------------------------------------------------

extern "C"
{
	COMMU_API
	BOOL
	DrvCheckDeviceExisted_X(
		char *		strVID,
		char *		strPID,
		char *		strSerNum,
		DWORD		dwPortNo
	);
	
	COMMU_API
	ERRCODE
	DrvUSBConnect_X(
		DWORD	dwUSBConnect_1,
		DWORD	dwUSBConnect_2,
		DWORD		dwPortNo
		);

	COMMU_API
	ERRCODE
	DrvClose_X(DWORD	dwPortNo);
	
	COMMU_API
	ERRCODE
	DrvSetControlData_X(
		DWORD		byControlID,
		DWORD 		dwValue,
		DWORD		dwSetControlData_2,
		DWORD		dwPortNo
	);


	COMMU_API
	ERRCODE
	DrvGetInformation_X(
		DWORD 		byParam,
		DWORD		*pdwvalue,
		DWORD		dwgetInformation_2,
		DWORD		dwPortNo
	);

	COMMU_API
	ERRCODE
	DrvReadData_X(
		BYTE 		*lpbuf, 
		DWORD 		dwLen,
		DWORD		dwReadData_1,
		DWORD		dwReadData_2,
		DWORD		dwPortNo
	);

	COMMU_API
	ERRCODE
	DrvWriteData_X(
		BYTE 		*lpbuf, 
		DWORD 		dwLen,
		DWORD		dwWriteData_1,
		DWORD		dwWRiteData_2,
		DWORD		dwPortNo
	);



	COMMU_API
	BOOL
	DrvCheckDeviceExisted(
		char *		strVID,
		char *		strPID
	);
	

	COMMU_API
	ERRCODE
	DrvUSBConnect(
		DWORD	dwUSBConnect_1,
		DWORD	dwUSBConnect_2
		);

	COMMU_API
	ERRCODE
	DrvClose();
	
	COMMU_API
	ERRCODE
	DrvSetControlData(
		DWORD		byControlID,
		DWORD 		dwValue,
		DWORD		dwSetControlData_2
	);


	COMMU_API
	ERRCODE
	DrvGetInformation(
		DWORD 		byParam,
		DWORD		*pdwvalue,
		DWORD		dwgetInformation_2
	);

	COMMU_API
	ERRCODE
	DrvReadData(
		BYTE 		*lpbuf, 
		DWORD 		dwLen,
		DWORD		dwReadData_1,
		DWORD		dwReadData_2
	);

	COMMU_API
	ERRCODE
	DrvWriteData(
		BYTE 		*lpbuf, 
		DWORD 		dwLen,
		DWORD		dwWriteData_1,
		DWORD		dwWRiteData_2
	);
} // extern "C"


#endif // __VENDORMASSSTOR_INTERFACE_H__
