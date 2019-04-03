#pragma once
#include <windows.h>
#include "Instdrv.h"
#include "shlwapi.h"

enum EInputType : unsigned long long
{
  eNone,
  eStartProtect,
  EStopProtect
};

struct SDriverInput
{
  EInputType type;
  unsigned long long reserved;
  unsigned int reserved2;
  unsigned int process_id;
};

constexpr unsigned int XXPROTECT_IOCODE = 0x222000;
const wchar_t* PROTECT_LINK_NAME = L"\\\\.\\SKProDriver";

class CProtect final
{
public:
  CProtect() = default;
  ~CProtect() = default;
  BOOL protect_process();
  BOOL unprotect_process() const;
private:


  HANDLE driver_handle = nullptr;
};


inline BOOL CProtect::protect_process() {
  wchar_t driver_path[MAX_PATH] = {0};
  GetModuleFileNameW(nullptr, driver_path, MAX_PATH);
  PathRemoveFileSpecW(driver_path);
  wcscat_s(driver_path, L"\\SKProDriver.sys");

  LoadNTDriver(static_cast<wchar_t *>(L"xxprotect"), driver_path, TRUE);
  const auto handle = CreateFile(PROTECT_LINK_NAME,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 nullptr,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 nullptr);

  if (handle == INVALID_HANDLE_VALUE) {
    return FALSE;
  }

  driver_handle = handle;
  SDriverInput input{eStartProtect, 0, 0, GetCurrentProcessId()};
  return DeviceIoControl(driver_handle, XXPROTECT_IOCODE, &input, sizeof(input), nullptr, 0, nullptr, nullptr);
}

inline BOOL CProtect::unprotect_process() const {
  if (!driver_handle)
    return FALSE;
  SDriverInput input{EStopProtect, 0, 0, GetCurrentProcessId()};
  DeviceIoControl(driver_handle, XXPROTECT_IOCODE, &input, sizeof(input), nullptr, 0, nullptr, nullptr);
  CloseHandle(driver_handle);
  UnloadDeviceDriver(L"xxprotect");
  return TRUE;
}
