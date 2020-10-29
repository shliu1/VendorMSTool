/********************************************************************************************/
//ComMassStor.cpp: Detect Mass Storage Connect by using Windows API
/********************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#include "VendorMSToolDlg.h"

#include <initguid.h>
#include <Setupapi.h>  


#define DWORD_PTR unsigned int*
#define ULONG_PTR unsigned long*
#define INTERFACE_DETAIL_SIZE    (1024)

#include "cfgmgr32.h"


#ifndef _GUID_DEVINTERFACE_VOLUME
DEFINE_GUID(_GUID_DEVINTERFACE_VOLUME,0x53f5630dL, 0xb6bf, 0x11d0, 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b);
DEFINE_GUID(UsbClassGuid, 0xa5dcbf10L, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed);
#endif


#if(defined UNICODE || _UNICODE)
	typedef wstring String;
#else
	typedef std::string String;
#endif

// removable devices info set
HDEVINFO g_hRemovableDeviceInfoSet = INVALID_HANDLE_VALUE;
static BOOLEAN PMPid=FALSE;
std::vector<CHAR> g_vtLetters;


// is device in the device info set
BOOL IsDeviceInDeviceInfoSet(HDEVINFO hDeviceInfoSet,DEVINST devInst)
{
	SP_DEVINFO_DATA info;
	ZeroMemory(&info,sizeof(info));
	info.cbSize = sizeof(info);

	DWORD index = 0;
	BOOL bRet = FALSE;
	for(; ; index ++)
	{
		// enum all devices
		if(!SetupDiEnumDeviceInfo(hDeviceInfoSet,index,&info))
			break;

		// compare devInst
		if(info.DevInst == devInst)
		{
			bRet = TRUE;
			break;
		}
	}

	return bRet;
}

// is a removable device with surprise removal set to FALSE
BOOL IsRemovableDevice(HDEVINFO hInfoSet,PSP_DEVINFO_DATA pInfo)
{
	
	BOOL bRet = FALSE;
	do
	{
		DWORD type = REG_DWORD;
		DWORD property;
		if(!SetupDiGetDeviceRegistryProperty(hInfoSet,
											 pInfo,
											 SPDRP_CAPABILITIES,
											 0,
											 (LPBYTE)&property,
											 sizeof(property),
											 0))
			break;

		// check it

		//if( (property & CM_DEVCAP_REMOVABLE) &&	!(property & (CM_DEVCAP_SURPRISEREMOVALOK | CM_DEVCAP_DOCKDEVICE)))
		if( (property & CM_DEVCAP_REMOVABLE))
			bRet = TRUE;



	}while(0);

	return bRet;
}




VOID CheckPMPid(TCHAR* idBuffer)
{
#if 1
	CString id(idBuffer);
			

	if(id.Left(7).CompareNoCase("USB\\VID")!=0)
		return ;

	if(id.Left(21).CompareNoCase(((CVendorMSToolDlg*)AfxGetMainWnd())->m_usbid)!=0)
	{
		PMPid=FALSE;
	}
	else
	{

		PMPid=TRUE;
	}
#endif
}




// device node to letter
VOID DeviceInst2Letter(DEVINST devInst,DWORD dwLevel,std::vector<TCHAR>& vtLetters)
{
	// 2k or later
	typedef BOOL (WINAPI *pfnGetVolumeNameForVolumeMountPoint)(LPVOID,LPVOID,DWORD);

	pfnGetVolumeNameForVolumeMountPoint pProc = 0;

	HMODULE hModule = 0;
	LPTSTR pBuffer = 0;
	TCHAR idBuffer[MAX_DEVICE_ID_LEN];
	ULONG len=0;

	__try
	{
		hModule = LoadLibrary(TEXT("kernel32.dll"));
		if(!hModule)
			__leave;

	#if(defined UNICODE || defined _UNICODE)
		pProc = (pfnGetVolumeNameForVolumeMountPoint)GetProcAddress(hModule,
										"GetVolumeNameForVolumeMountPointW");
	#else
		pProc = (pfnGetVolumeNameForVolumeMountPoint)GetProcAddress(hModule,
										"GetVolumeNameForVolumeMountPointA");
	#endif
	
		if(!pProc)
			__leave;
		
		// get device id first
		if(CR_SUCCESS != CM_Get_Device_ID_Ex(devInst,
											 idBuffer,
											 sizeof(idBuffer)/sizeof(TCHAR),
											 0,0))
			__leave;

		// get interface list size for the device
		if(CR_SUCCESS != CM_Get_Device_Interface_List_Size(&len,
														   (LPGUID)&_GUID_DEVINTERFACE_VOLUME,
														   idBuffer,
														   0))
			__leave;


		if(!len)
			__leave;


		CheckPMPid(idBuffer);

		pBuffer = new TCHAR[len];
		LPTSTR pTemp = pBuffer;

		// get interface list
		if(CR_SUCCESS != CM_Get_Device_Interface_List_Ex((LPGUID)&_GUID_DEVINTERFACE_VOLUME,
														 idBuffer,
														 pBuffer,
														 len,
														 0,
														 0))
			__leave;

		size_t i = 0;
		while(i < len && *pTemp)
		{
			TCHAR szDeviceName[MAX_PATH];
			_tcscpy(szDeviceName,pTemp);

			// add a trailing backslash
			LPTSTR pEnd = _tcschr(szDeviceName + 4 * sizeof(TCHAR),TEXT('\\'));
			if(!pEnd)
				_tcscat(szDeviceName,TEXT("\\"));

			TCHAR szMountFromDevice[MAX_PATH];

			// get the mount point name
			if(!pProc(szDeviceName,szMountFromDevice,
					  sizeof(szMountFromDevice)/sizeof(TCHAR)))
				__leave;

			// from a to z get mount point name
			TCHAR szLetter[] = TEXT("A:\\");
			for( ;szLetter[0] <= TEXT('Z'); szLetter[0] = szLetter[0] + 1)
			{
				TCHAR szMountFromLetter[MAX_PATH];

				if(pProc(szLetter,szMountFromLetter,
						 sizeof(szMountFromLetter)/sizeof(TCHAR)))

				{
					// check is the same
					if(_tcscmp(szMountFromLetter,szMountFromDevice) == 0)
					{
						if(PMPid)
							vtLetters.push_back(szLetter[0]);
						//__leave;
					}
				}
			}

			// next sym link name
			i += _tcslen(pTemp) + 1;
			pTemp += _tcslen(pTemp) + 1;
		}
	}
	__finally
	{
		if(hModule)
			FreeLibrary(hModule);

		if(pBuffer)
			delete[] pBuffer;
	}

	__try
	{
		
	}
	__finally
	{
	}

	LPTSTR pRemvalRelDevicesIdBuffer = 0;
	__try
	{
		// send query removal relation irp
		if(CR_SUCCESS != CM_Get_Device_ID_List_Size(&len,
													idBuffer,
													CM_GETIDLIST_FILTER_REMOVALRELATIONS))
			__leave;

		if(!len)
			__leave;

		pRemvalRelDevicesIdBuffer = new TCHAR[len];

		if(CR_SUCCESS != CM_Get_Device_ID_List(idBuffer,
											   pRemvalRelDevicesIdBuffer,
											   len,
											   CM_GETIDLIST_FILTER_REMOVALRELATIONS))
			__leave;

		LPTSTR pTemp = pRemvalRelDevicesIdBuffer;

		// for each device get its drive letter
		for(size_t i = 0; i < len && *pTemp;)
		{
			DEVINST devInst;
			if(CR_SUCCESS == CM_Locate_DevNode(&devInst,pTemp,CM_LOCATE_DEVNODE_NORMAL))
			{
				VOID GetDriveLettersForDevice(DEVINST,DWORD,std::vector<TCHAR>&);
				GetDriveLettersForDevice(devInst,dwLevel + 1,vtLetters);
			}

			pTemp += _tcslen(pTemp) + 1;
			i+= _tcslen(pTemp) + 1;
		}
	}
	__finally
	{
		if(pRemvalRelDevicesIdBuffer)
			delete[] pRemvalRelDevicesIdBuffer;
	}
}


// get drive letter
VOID GetDriveLettersForDevice(DEVINST devInst,DWORD dwLevel,std::vector<TCHAR>& vtLetters)
{
	__try
	{
		// get self first
		DeviceInst2Letter(devInst,dwLevel,vtLetters);
		
		// for all children
		DEVINST devChild,devNext;

		if(CR_SUCCESS != CM_Get_Child(&devChild,devInst,0))
			__leave;
			
		for(;;)
		{
			// do not check removable child device
			if(!IsDeviceInDeviceInfoSet(g_hRemovableDeviceInfoSet,devChild))
			{
				// for child
				GetDriveLettersForDevice(devChild,dwLevel + 1,vtLetters);
			}

			// get sibling
			if(CR_SUCCESS != CM_Get_Sibling(&devNext,devChild,0))
				__leave;

			devChild = devNext;
		}
	}
	__finally
	{
		// nothing to do
	}
}




// get device desc
VOID GetDeviceDesc(HDEVINFO hInfoSet,PSP_DEVINFO_DATA pInfo,LPTSTR pszBuffer,DWORD len)
{
	std::vector<TCHAR> vtLetters;  //FK

	// get letters
	GetDriveLettersForDevice(pInfo->DevInst,1,g_vtLetters);

	// get device friendly name
	DWORD type = REG_SZ;
	DWORD size = 0;
	LPBYTE pFriendlyName = 0;
	if(SetupDiGetDeviceRegistryProperty(hInfoSet,
										pInfo,
										SPDRP_FRIENDLYNAME,
										0,
										0,
										0,
										&size) || 
	   GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		pFriendlyName = new BYTE[size];
		ZeroMemory(pFriendlyName,size);

		SetupDiGetDeviceRegistryProperty(hInfoSet,
										pInfo,
										SPDRP_FRIENDLYNAME,
										0,
										pFriendlyName,
										size,
										0);
	}

	// get device desc
	if(!pFriendlyName || !*pFriendlyName)
	{
		if(pFriendlyName)
			delete[] pFriendlyName;

		pFriendlyName = 0;

		if(SetupDiGetDeviceRegistryProperty(hInfoSet,
											pInfo,
											SPDRP_DEVICEDESC,
											0,
											0,
											0,
											&size) || 
		   GetLastError() == ERROR_INSUFFICIENT_BUFFER)
		{
			pFriendlyName = new BYTE[size];
			ZeroMemory(pFriendlyName,size);

			SetupDiGetDeviceRegistryProperty(hInfoSet,
											pInfo,
											SPDRP_DEVICEDESC,
											0,
											pFriendlyName,
											size,
											0);
		}
	}

	if(!pFriendlyName || !*pFriendlyName)
	{
		if(pFriendlyName)
			delete[] pFriendlyName;

		pFriendlyName = 0;
	}

	if(!g_vtLetters.empty())
	{
		_sntprintf(pszBuffer,len,TEXT("%s - Drive ("),
				   pFriendlyName ? (LPCTSTR)pFriendlyName : TEXT("Device"));
	  
		String strBuffer = pszBuffer;

		for(size_t i = 0; i < g_vtLetters.size(); i ++)
		{
			strBuffer += g_vtLetters[i];
			strBuffer += TEXT(":");
			if(i == g_vtLetters.size() - 1)
				strBuffer += TEXT(")");
			else
				strBuffer += TEXT(",");
		}

		_tcsncpy(pszBuffer,strBuffer.c_str(),len);
	}
	else
	{
		_sntprintf(pszBuffer,len,TEXT("%s"), pFriendlyName ? (LPCTSTR)pFriendlyName : TEXT("Device"));
	}

	if(pFriendlyName)
		delete[] pFriendlyName;
}



int CheckPMPdevice()
{
	HDEVINFO hAllDevicesInfoSet = INVALID_HANDLE_VALUE;
	PMPid=FALSE;

	__try
	{
		// get all devices info set  
		hAllDevicesInfoSet = SetupDiGetClassDevs(0,0,0,DIGCF_ALLCLASSES|DIGCF_PRESENT);  
		//returns a device information set that contains info on all installed devices of a specified class.   

		if(hAllDevicesInfoSet == INVALID_HANDLE_VALUE)
			__leave;

		// create an empty device info set,we will insert all removable devices into it
		g_hRemovableDeviceInfoSet = SetupDiCreateDeviceInfoList(0,0);

		if(g_hRemovableDeviceInfoSet == INVALID_HANDLE_VALUE)
			__leave;

		DWORD index = 0;
		for(; ;index ++)
		{
			// enum all devices
			SP_DEVINFO_DATA info;
			ZeroMemory(&info,sizeof(info));
			info.cbSize = sizeof(info);

			if(!SetupDiEnumDeviceInfo(hAllDevicesInfoSet,index,&info))
				break;
			
			// is already in our removable device info set?
			if(IsDeviceInDeviceInfoSet(g_hRemovableDeviceInfoSet,info.DevInst))
				continue;
			
			// is a removable device and surprise removal = FALSE

			if(!IsRemovableDevice(hAllDevicesInfoSet,&info))
				continue;

			TCHAR idBuffer[MAX_DEVICE_ID_LEN];
			if(!SetupDiGetDeviceInstanceId(hAllDevicesInfoSet,
										   &info,
										   idBuffer,
										   sizeof(idBuffer) / sizeof(TCHAR),
										   0))
				continue;
			
			_tprintf(TEXT("Instance id %s\n\n"),idBuffer);
			
			//   \\?\Volume{36FC9E60-C465-11CF-8056-444553540000}\. 
			// this will add it to the removable device info set
			if(!SetupDiOpenDeviceInfo(g_hRemovableDeviceInfoSet,idBuffer,0,0,0))
				continue;
		}

		// for each removable device get its description
		index = 0;
		for(;;index ++)	
		{
			// enum all devices
			SP_DEVINFO_DATA info;
			ZeroMemory(&info,sizeof(info));
			info.cbSize = sizeof(info);

			if(!SetupDiEnumDeviceInfo(g_hRemovableDeviceInfoSet,index,&info))
				break;

			TCHAR idBuffer[MAX_DEVICE_ID_LEN];

			if(!SetupDiGetDeviceInstanceId(g_hRemovableDeviceInfoSet,
										   &info,
										   idBuffer,
										   sizeof(idBuffer) / sizeof(TCHAR),
										   0))
				continue;
			
			// get device desc
			GetDeviceDesc(g_hRemovableDeviceInfoSet,
						  &info,
						  idBuffer,
						  sizeof(idBuffer) / sizeof(TCHAR));
			
			_tprintf(TEXT("Device Descriptor %s\n"),idBuffer);
		//	printf(idBuffer);

		}
	}
	__finally
	{
		if(hAllDevicesInfoSet != INVALID_HANDLE_VALUE)
			SetupDiDestroyDeviceInfoList(hAllDevicesInfoSet);

		if(g_hRemovableDeviceInfoSet != INVALID_HANDLE_VALUE)
			SetupDiDestroyDeviceInfoList(g_hRemovableDeviceInfoSet);
	}


	return 0;
}




int GetDevicePath(LPGUID lpGuid, LPTSTR* pszDevicePath)
{
    HDEVINFO hDevInfoSet;   
    SP_DEVICE_INTERFACE_DATA ifdata;
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDetail;
    int nCount;
    BOOL bResult;
 
    
    hDevInfoSet = ::SetupDiGetClassDevs((LPGUID)&_GUID_DEVINTERFACE_VOLUME,     // class GUID 
        NULL,    
        NULL,                 
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);   

    if (hDevInfoSet == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
 

    pDetail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)::GlobalAlloc(LMEM_ZEROINIT, INTERFACE_DETAIL_SIZE);
 
    pDetail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
 
    nCount = 0;
    bResult = TRUE;

    while (bResult)
    {
        ifdata.cbSize = sizeof(ifdata);

        bResult = ::SetupDiEnumDeviceInterfaces(
            hDevInfoSet,    
            NULL,           
            lpGuid,         
            (ULONG)nCount,  
            &ifdata);        
 
        if (bResult)
        {
           
            bResult = SetupDiGetInterfaceDeviceDetail(
                hDevInfoSet,   
                &ifdata,      
                pDetail,        
                INTERFACE_DETAIL_SIZE,   
                NULL,          
                NULL);         
            if (bResult)
            {
              
                ::strcpy(pszDevicePath[nCount], pDetail->DevicePath);
             
                nCount++;
            }
        }
    }

    ::GlobalFree(pDetail);

    ::SetupDiDestroyDeviceInfoList(hDevInfoSet);
    return nCount;
}

