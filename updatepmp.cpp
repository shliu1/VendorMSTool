/********************************************************************************************/
//updatepmp.cpp : Defines the entry point for the DLL application.
/********************************************************************************************/

#include "stdafx.h"

//extern "C"
//{

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <setupapi.h>
#include <initguid.h>
#include <stddef.h>
#include <stdlib.h>
#include "Resource.h"
#include "devioctl.h"
#include "ntdddisk.h"
#include "ntddscsi.h"
#include "spti.h"
#include "VendorMSToolDlg.h"

//}

extern DWORD g_dwUSBUBPAddr;
extern BOOL  g_bRunFA93;

typedef struct _DRIVER_MAP
{
	UINT nID;
	TCHAR name[64];

} DRIVER_MAP;



#define FSCTL_LOCK_VOLUME               CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  6, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_UNLOCK_VOLUME             CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  7, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DISMOUNT_VOLUME           CTL_CODE(FILE_DEVICE_FILE_SYSTEM,  8, METHOD_BUFFERED, FILE_ANY_ACCESS)




BOOL OpenPMPDevice(CHAR* name,PHANDLE handle)
{
	
    *handle = CreateFile ( name,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,					// no SECURITY_ATTRIBUTES structure
                        OPEN_EXISTING,			// No special create flags
                        0,						// No special attributes
                        NULL);					// No template file

    if (INVALID_HANDLE_VALUE == *handle) {

		*handle=NULL;
        return FALSE;
    }

    return TRUE;

}



BOOL CMD_IbrWrite( HANDLE w_handle,LPVOID lpBuffer,int file_len)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	DWORD nNumberOfBytesToWrite;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;
	
	nNumberOfBytesToWrite=file_len;

	sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite;

    sptdwb.sptd.TimeOutValue = 50;

	sptdwb.sptd.DataBuffer = lpBuffer;

    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x72;
	sptdwb.sptd.Cdb[2] = file_len&0xFF;
	sptdwb.sptd.Cdb[3] = (file_len>>8)&0xFF;
	sptdwb.sptd.Cdb[4] = (file_len >>16) & 0xFF;
	sptdwb.sptd.Cdb[5] = 0;
	
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);

    status = DeviceIoControl(w_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);


	//PrintError(GetLastError(),__FILE__,__LINE__);

	return (status != 0);

}


VOID Close_IC_Handle(HANDLE handle)
{
	if(handle)
		CloseHandle(handle);
}

HANDLE Open_IC_Handle(CHAR* name)
{
	HANDLE mass_handle=NULL;
	DWORD dwBytesReturned = 0;
	DWORD res=0;
	
	if(!OpenPMPDevice(name,&mass_handle))
	{
		return 0;
	}

	return mass_handle;
}



BOOL CMD_IbrConnect(HANDLE r_handle,CHAR* data,int arg)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;


	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = 0xdff;  



    sptdwb.sptd.TimeOutValue = 20;

	sptdwb.sptd.DataBuffer = data;

    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	
    sptdwb.sptd.Cdb[0] = 0x6;
    sptdwb.sptd.Cdb[1] = 0x51; 
	sptdwb.sptd.Cdb[2] = arg;  //0x1=> Open  0x2=> Close

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}



BOOL CMD_IbrSetaddress(HANDLE r_handle,CHAR* data)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = 0xdff;  
    sptdwb.sptd.TimeOutValue = 20;
	sptdwb.sptd.DataBuffer = data;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
/*
#if 1	
    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x55; 
	sptdwb.sptd.Cdb[2] = 0x00;
	sptdwb.sptd.Cdb[3] = 0x60;
	sptdwb.sptd.Cdb[4] = 0xE7;
	sptdwb.sptd.Cdb[5] = 0xFF;
#else
	sptdwb.sptd.Cdb[0] = 0x00;
    sptdwb.sptd.Cdb[1] = 0x00; 
	sptdwb.sptd.Cdb[2] = 0x00;
	sptdwb.sptd.Cdb[3] = 0x00;
	sptdwb.sptd.Cdb[4] = 0x00;
	sptdwb.sptd.Cdb[5] = 0xFF;

#endif
  */
    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x55; 
	sptdwb.sptd.Cdb[2] = (BYTE)(g_dwUSBUBPAddr);
	sptdwb.sptd.Cdb[3] = (BYTE)(g_dwUSBUBPAddr >> 8);
	sptdwb.sptd.Cdb[4] = (BYTE)(g_dwUSBUBPAddr >> 16);
	sptdwb.sptd.Cdb[5] = (BYTE)(g_dwUSBUBPAddr >> 24);
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}


BOOL CMD_IbrExecute(HANDLE r_handle,CHAR* data)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 0; //24;   // because booting must have sense, so we set 0 to obtain the few time.
	if ( g_bRunFA93 == TRUE )
	{
	   sptdwb.sptd.DataTransferLength = 0; //0xdff;  
    }
	else
	{
	   sptdwb.sptd.DataTransferLength = 8; //0xdff; 
	}
    sptdwb.sptd.TimeOutValue = 5;
	sptdwb.sptd.DataBuffer = data;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	
    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x60; 

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);


//	return (status != 0);
//	Sleep(1000);
	return(1);
}



BOOL CMD_WriterProcess( HANDLE w_handle,unsigned int* data,int parameter0,int nByteToWrite,int LUN_ID)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	DWORD nNumberOfBytesToWrite;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;

	nNumberOfBytesToWrite= nByteToWrite;
	sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite;
    sptdwb.sptd.TimeOutValue = 100;
	sptdwb.sptd.DataBuffer = (PVOID)data;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    
	sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_SendInformation;
	sptdwb.sptd.Cdb[1] = LUN_ID;
    sptdwb.sptd.Cdb[6] = parameter0;
	sptdwb.sptd.Cdb[7] = 0x0;
 

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(w_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);


	return (status != 0);

}


INT CMD_WriteFile( HANDLE w_handle, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite, 
				 LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped,int LUN_ID)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	WORD wTwoByte;

	if (nNumberOfBytesToWrite == 1)
		wTwoByte = *(WORD *)lpBuffer;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;

	if (nNumberOfBytesToWrite == 1)
		sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite+1;
	else
		sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite;

    sptdwb.sptd.TimeOutValue = 20;

	if (nNumberOfBytesToWrite == 1)
		sptdwb.sptd.DataBuffer = &wTwoByte;
	else
		sptdwb.sptd.DataBuffer = lpBuffer;

    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_Write;
	sptdwb.sptd.Cdb[1] = LUN_ID;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(w_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);


	if (status!=0)
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;	
	else
		*lpNumberOfBytesWritten = 0;

	if (status != 0)
		return 1;
	else 
		return 0;

}


BOOL CMD_GetImageList(HANDLE r_handle,CHAR* list,int nByteToGet,int arg0,int arg1,int LUN_ID)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = nByteToGet;  
	sptdwb.sptd.TimeOutValue = 50;
	sptdwb.sptd.DataBuffer = list;
	
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	
    sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_GetImageList;
	sptdwb.sptd.Cdb[1] = LUN_ID;
	
	sptdwb.sptd.Cdb[6] = arg0;
	sptdwb.sptd.Cdb[7] = arg1;

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}


BOOL CMD_Verify(HANDLE r_handle,CHAR* data,int nByteToGet,int LUN_ID)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = nByteToGet;  
    sptdwb.sptd.TimeOutValue = 20;
	sptdwb.sptd.DataBuffer = data;

    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	
    sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_Verify;
	sptdwb.sptd.Cdb[1] = LUN_ID;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}

BOOL CMD_GetInfo(HANDLE r_handle,CHAR* data,int arg0,int LUN_ID)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24; 
    if ( GET_NAND_FLASH_ID == arg0 )
		sptdwb.sptd.DataTransferLength =5;  
	else
		sptdwb.sptd.DataTransferLength =4;  
    sptdwb.sptd.TimeOutValue = 20;
	sptdwb.sptd.DataBuffer = data;
	
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
	
    sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_GetInformation;
	sptdwb.sptd.Cdb[6] = arg0;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}



INT CMD_SetInformation( HANDLE w_handle, LPVOID lpBuffer, DWORD nNumberOfBytesToWrite, 
				 LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped,int parameter)
{
	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	WORD wTwoByte;

	if (nNumberOfBytesToWrite == 1)
		wTwoByte = *(WORD *)lpBuffer;

	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));
    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.SenseInfoLength = 24;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_OUT;

	if (nNumberOfBytesToWrite == 1)
		sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite+1;
	else
		sptdwb.sptd.DataTransferLength = nNumberOfBytesToWrite;

    sptdwb.sptd.TimeOutValue = 100;

	if (nNumberOfBytesToWrite == 1)
		sptdwb.sptd.DataBuffer = &wTwoByte;
	else
		sptdwb.sptd.DataBuffer = lpBuffer;

    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);
    sptdwb.sptd.Cdb[0] = SCSIOP_VENDOR_SetInformation;
	sptdwb.sptd.Cdb[6] = parameter;
    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(w_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	if (status!=0)
		*lpNumberOfBytesWritten = nNumberOfBytesToWrite;	
	else
		*lpNumberOfBytesWritten = 0;

	if (status != 0)
		return 1;
	else 
		return 0;

}

// Ray added it for FA93's clock skew
BOOL CMD_IbrSetDQSODS(HANDLE r_handle,CHAR* data, DWORD dwDQSODS)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = 0xdff;  
    sptdwb.sptd.TimeOutValue = 20;
	sptdwb.sptd.DataBuffer = data;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);

    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x52; 
	sptdwb.sptd.Cdb[2] = dwDQSODS & 0x000000FF;
	sptdwb.sptd.Cdb[3] = (dwDQSODS & 0x0000FF00) >> 8;
	sptdwb.sptd.Cdb[4] = (dwDQSODS & 0x00FF0000) >> 16;
	sptdwb.sptd.Cdb[5] = (dwDQSODS & 0xFF000000) >> 24;

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}

BOOL CMD_IbrSetCKDQSDS(HANDLE r_handle,CHAR* data, DWORD dwCKDQSDS)
{

	SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER sptdwb;
	ULONG length = 0, returned = 0;
	BOOL status = 0;
	ZeroMemory(&sptdwb, sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER));

    sptdwb.sptd.Length = sizeof(SCSI_PASS_THROUGH_DIRECT);
    sptdwb.sptd.PathId = 0;
    sptdwb.sptd.TargetId = 1;
    sptdwb.sptd.Lun = 0;
    sptdwb.sptd.CdbLength = CDB10GENERIC_LENGTH;
    sptdwb.sptd.DataIn = SCSI_IOCTL_DATA_IN;
    sptdwb.sptd.SenseInfoLength = 24;      
	sptdwb.sptd.DataTransferLength = 0xdff;  
    sptdwb.sptd.TimeOutValue = 20;
	sptdwb.sptd.DataBuffer = data;
    sptdwb.sptd.SenseInfoOffset =
       offsetof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER,ucSenseBuf);

    sptdwb.sptd.Cdb[0] = 0x06;
    sptdwb.sptd.Cdb[1] = 0x53; 
	sptdwb.sptd.Cdb[2] = dwCKDQSDS & 0x000000FF;
	sptdwb.sptd.Cdb[3] = (dwCKDQSDS & 0x0000FF00) >> 8;
	sptdwb.sptd.Cdb[4] = (dwCKDQSDS & 0x00FF0000) >> 16;
	sptdwb.sptd.Cdb[5] = (dwCKDQSDS & 0xFF000000) >> 24;

    length = sizeof(SCSI_PASS_THROUGH_DIRECT_WITH_BUFFER);
    status = DeviceIoControl(r_handle,
                             IOCTL_SCSI_PASS_THROUGH_DIRECT,
                             &sptdwb,
                             length,
                             &sptdwb,
                             length,
                             &returned,
                             FALSE);

	return (status != 0);
}