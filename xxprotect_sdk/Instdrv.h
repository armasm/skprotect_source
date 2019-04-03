#ifndef _INSTDRV_H_
#define _INSTDRV_H_

BOOL 
LoadDeviceDriver( 
	IN const TCHAR * Name, 
	IN const TCHAR * Path, 
	OUT HANDLE * lphDevice, 
	OUT PDWORD Error 
	);

BOOL 
UnloadDeviceDriver( 
	IN const TCHAR * Name
	);

BOOL 
LoadNTDriver(
	PWCHAR lpszDriverName, 
	PWCHAR lpszDriverPath, 
	BOOL bForceReload
	);

BOOL 
UnloadNTDriver(
	PWCHAR wszSvrName
	);

#endif // _INSTDRV_H_
