/******************************************************************************
*
*       FileMon - File System Monitor for Windows NT/9x
*		
*		Copyright (c) 1996 Mark Russinovich and Bryce Cogswell
*
*		See readme.txt for terms and conditions.
*
*    	PROGRAM: Instdrv.c
*
*    	PURPOSE: Loads and unloads the Filemon device driver. This code
*		is taken from the instdrv example in the NT DDK.
*
******************************************************************************/
 #include <windows.h>
 #include <stdlib.h>
 #include <string.h>


//#include "..\CommonHelper\helper.h"

/****************************************************************************
*
*    FUNCTION: InstallDriver( IN SC_HANDLE, IN LPCTSTR, IN LPCTSTR)
*
*    PURPOSE: Creates a driver service.
*
****************************************************************************/
BOOL 
InstallDriver(
	IN SC_HANDLE SchSCManager, 
	IN LPCTSTR DriverName, 
	IN LPCTSTR ServiceExe
	)
{
    SC_HANDLE schService;

    //
    // NOTE: This creates an entry for a standalone driver. If this
    //       is modified for use with a driver that requires a Tag,
    //       Group, and/or Dependencies, it may be necessary to
    //       query the registry for existing driver information
    //       (in order to determine a unique Tag, etc.).
    //

    schService = CreateService( 
					SchSCManager,           // SCManager database
					DriverName,             // name of service
					DriverName,             // name to display
					SERVICE_ALL_ACCESS,     // desired access
					SERVICE_KERNEL_DRIVER,	// service type
					SERVICE_AUTO_START,     // start type
					SERVICE_ERROR_NORMAL,   // error control type
					ServiceExe,             // service's binary
					NULL,                   // no load ordering group
					NULL,                   // no tag identifier
					NULL,                   // no dependencies
					NULL,                   // LocalSystem account
					NULL                    // no password
					);

    if (schService == NULL)
        return FALSE;

    CloseServiceHandle(schService);

    return TRUE;
}


/****************************************************************************
*
*    FUNCTION: StartDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Starts the driver service.
*
****************************************************************************/
BOOL 
StartDriver(
	IN SC_HANDLE SchSCManager, 
	IN LPCTSTR DriverName
	)
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( 
					SchSCManager,
					DriverName,
					SERVICE_ALL_ACCESS
					);

    if (schService == NULL)
        return FALSE;

    ret = StartService(schService, 0, NULL)
       || GetLastError() == ERROR_SERVICE_ALREADY_RUNNING 
	   || GetLastError() == ERROR_SERVICE_DISABLED;

    CloseServiceHandle(schService);

    return ret;
}



/****************************************************************************
*
*    FUNCTION: OpenDevice( IN LPCTSTR, HANDLE *)
*
*    PURPOSE: Opens the device and returns a handle if desired.
*
****************************************************************************/
BOOL 
OpenDevice( 
	IN LPCTSTR DriverName, 
	OUT HANDLE * lphDevice 
	)
{
    TCHAR completeDeviceName[64];
    HANDLE hDevice;

    //
    // Create a \\.\XXX device name that CreateFile can use
    //
    // NOTE: We're making an assumption here that the driver
    //       has created a symbolic link using it's own name
    //       (i.e. if the driver has the name "XXX" we assume
    //       that it used IoCreateSymbolicLink to create a
    //       symbolic link "\DosDevices\XXX". Usually, there
    //       is this understanding between related apps/drivers.
    //
    //       An application might also peruse the DEVICEMAP
    //       section of the registry, or use the QueryDosDevice
    //       API to enumerate the existing symbolic links in the
    //       system.
    //

	if((GetVersion() & 0xFF) >= 5) 
	{
		//
		// We reference the global name so that the application can
		// be executed in Terminal Services sessions on Win2K
		//
		wsprintf(completeDeviceName, TEXT("\\\\.\\Global\\%s"), DriverName);
	} 
	else 
	{
		wsprintf(completeDeviceName, TEXT("\\\\.\\%s"), DriverName);
	}
    hDevice = CreateFile( 
				completeDeviceName,
				GENERIC_READ | GENERIC_WRITE,
				0,
				NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
				);

    if (hDevice == ((HANDLE)-1))
        return FALSE;

	// If user wants handle, give it to them.  Otherwise, just close it.
	if (lphDevice)
		*lphDevice = hDevice;
	else
	    CloseHandle(hDevice);

    return TRUE;
}



/****************************************************************************
*
*    FUNCTION: StopDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Has the configuration manager stop the driver (unload it)
*
****************************************************************************/
BOOL 
StopDriver(
	IN SC_HANDLE SchSCManager, 
	IN LPCTSTR DriverName
	)
{
    SC_HANDLE       schService;
    BOOL            ret;
    SERVICE_STATUS  serviceStatus;

    schService = OpenService(SchSCManager, DriverName, SERVICE_ALL_ACCESS);
    if (schService == NULL)
        return FALSE;

    ret = ControlService(schService, SERVICE_CONTROL_STOP, &serviceStatus);

    CloseServiceHandle(schService);

    return ret;
}


/****************************************************************************
*
*    FUNCTION: RemoveDriver( IN SC_HANDLE, IN LPCTSTR)
*
*    PURPOSE: Deletes the driver service.
*
****************************************************************************/
BOOL 
RemoveDriver( 
	IN SC_HANDLE SchSCManager, 
	IN LPCTSTR DriverName 
	)
{
    SC_HANDLE  schService;
    BOOL       ret;

    schService = OpenService( 
					SchSCManager,
					DriverName,
					SERVICE_ALL_ACCESS
					);

    if (schService == NULL)
	{
        return FALSE;
	}

    ret = DeleteService(schService);

    CloseServiceHandle(schService);

    return ret;
}


/****************************************************************************
*
*    FUNCTION: UnloadDeviceDriver( const TCHAR *)
*
*    PURPOSE: Stops the driver and has the configuration manager unload it.
*
****************************************************************************/
BOOL 
UnloadDeviceDriver( 
	IN const TCHAR * Name
	)
{
    BOOL bRet = FALSE;
	SC_HANDLE	schSCManager = NULL;

    do 
    {
	    schSCManager = OpenSCManager(	
							NULL,                 // machine (NULL == local)
							NULL,                 // database (NULL == default)
							SC_MANAGER_ALL_ACCESS // access required
							);

        if (schSCManager == NULL)
        {
            break;
        }

	    bRet = StopDriver(schSCManager, Name);
        if (!bRet)
        {
            break;
        }

	    bRet = RemoveDriver(schSCManager, Name);

    } while(FALSE);
	
    if (schSCManager)
    {
	    CloseServiceHandle(schSCManager);
    }

	return bRet;
}



/****************************************************************************
*
*    FUNCTION: LoadDeviceDriver( const TCHAR, const TCHAR, HANDLE *)
*
*    PURPOSE: Registers a driver with the system configuration manager 
*	 and then loads it.
*
****************************************************************************/
BOOL 
LoadDeviceDriver( 
	IN const TCHAR * Name, 
	IN const TCHAR * Path, 
	OUT HANDLE * lphDevice, 
	OUT PDWORD Error 
	)
{
	SC_HANDLE schSCManager;
	BOOL okay;

	schSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	// Remove previous instance
	RemoveDriver(schSCManager, Name);

	// Ignore success of installation: it may already be installed.
	okay = InstallDriver(schSCManager, Name, Path);
	if (!okay)
	{
		RemoveDriver(schSCManager, Name);
		*Error = GetLastError();

		return okay;
	}

	// Ignore success of start: it may already be started.
	okay = StartDriver(schSCManager, Name);
	if (!okay)
	{
		RemoveDriver(schSCManager, Name);
		*Error = GetLastError();

		return okay;
	}

	// Do make sure we can open it.
	okay = OpenDevice(Name, lphDevice);
	if (!okay)
	{
		BOOL bStop = FALSE;
		bStop = StopDriver(schSCManager, Name);
		if (!bStop)
		{
			return okay;
		}
		RemoveDriver(schSCManager, Name);
		*Error = GetLastError();

		return okay;
	}

 	CloseServiceHandle(schSCManager);

	return okay;
}

//卸载驱动程序  
BOOL 
UnloadNTDriver(
	PWCHAR wszSvrName
	)
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄
	SERVICE_STATUS SvrStatus;

	//打开SCM管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if(NULL == hServiceMgr)
	{
		//打开SCM管理器失败
		bRet = FALSE;
		goto BeforeLeave;
	}

	//打开驱动所对应的服务
	hServiceDDK = OpenServiceW(hServiceMgr, wszSvrName, SERVICE_ALL_ACCESS);
	if(NULL == hServiceDDK)
	{
		//打开驱动所对应的服务失败
		bRet = FALSE;
		goto BeforeLeave;
	}

	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。  
	ControlService(hServiceDDK, SERVICE_CONTROL_STOP , &SvrStatus);

	//动态卸载驱动程序。  
	DeleteService(hServiceDDK);

	bRet = TRUE;

BeforeLeave:
	//离开前关闭打开的句柄
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

	return bRet;	
}

//加载驱动程序
BOOL 
LoadNTDriver(
	PWCHAR lpszDriverName, 
	PWCHAR lpszDriverPath, 
	BOOL bForceReload
	)
{
	WCHAR szDriverImagePath[MAX_PATH] = {0};
        DWORD dwRtn = 0;
   

	//得到完整的驱动路径, 该调用可以确定lpszDriverPath中的文件确实已经存在了
	GetFullPathNameW(lpszDriverPath, MAX_PATH, szDriverImagePath, NULL);

	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;//SCM管理器的句柄
	SC_HANDLE hServiceDDK = NULL;//NT驱动程序的服务句柄

	//如果强制重新加载, 先卸载删除服务, 再重新加载
	if (bForceReload)
	{
		UnloadNTDriver(lpszDriverName);
	}

	//打开服务控制管理器
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

	if(NULL == hServiceMgr)  
	{
     
		//OpenSCManager失败
		bRet = FALSE;
		goto BeforeLeave;
	}

 

	//创建驱动所对应的服务
	hServiceDDK = CreateServiceW(
					hServiceMgr,
					lpszDriverName,			//驱动程序的在注册表中的名字
					lpszDriverName,			// 注册表驱动程序的 DisplayName 值
					SERVICE_ALL_ACCESS,		// 加载驱动程序的访问权限
					SERVICE_KERNEL_DRIVER,	// 表示加载的服务是驱动程序
					SERVICE_DEMAND_START,	// 注册表驱动程序的 Start 值
					SERVICE_ERROR_IGNORE,	// 注册表驱动程序的 ErrorControl 值
					szDriverImagePath,		// 注册表驱动程序的 ImagePath 值
					NULL,  
					NULL,  
					NULL,  
					NULL,  
					NULL
					);  

	

	//判断服务是否失败
	if(NULL == hServiceDDK)
	{
		dwRtn = GetLastError();
		if(ERROR_IO_PENDING != dwRtn 
			&& ERROR_SERVICE_EXISTS != dwRtn)
		{
           

			//由于其他原因创建服务失败
			bRet = FALSE;
			goto BeforeLeave;
		}

		// 驱动程序已经加载，只需要打开  
		hServiceDDK = OpenServiceW(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if(NULL == hServiceDDK)
		{
			//如果打开服务也失败，则意味错误
			dwRtn = GetLastError();
           

			bRet = FALSE;
			goto BeforeLeave;
		}
	}

	//开启此项服务
	bRet= StartServiceW(hServiceDDK, NULL, NULL);
	if(!bRet)
	{
		DWORD dwRtn = GetLastError();
		if(ERROR_IO_PENDING != dwRtn 
			&& ERROR_SERVICE_ALREADY_RUNNING != dwRtn)
		{ 
            //DBG_TRACE_PRINT(TRACE_CHECK, ("%s StartService failed Error: %x.\n", __FUNCTION__, dwRtn));
			bRet = FALSE;
			goto BeforeLeave;
		} 
		else 
		{ 
			if(ERROR_IO_PENDING == dwRtn) 
			{ 
               // DBG_TRACE_PRINT(TRACE_CHECK, ("%s StartService failed Error: %x.\n", __FUNCTION__, dwRtn));
				//设备被挂住
				bRet = FALSE;
				goto BeforeLeave;
			} 
			else 
			{ 
              //  DBG_TRACE_PRINT(TRACE_CHECK, ("%s StartService succeed.\n", __FUNCTION__));
				//服务已经开启
				bRet = TRUE;
				goto BeforeLeave;
			} 
		} 
	}
	bRet = TRUE;

	//离开前关闭句柄
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}

    //DBG_TRACE_PRINT(TRACE_CHECK, ("%s leave.\n", __FUNCTION__));

	return bRet;
}