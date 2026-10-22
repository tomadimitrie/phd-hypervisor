#pragma once

#ifdef _KERNEL_MODE
# include <ntddk.h>
#else
# include <Windows.h>
#endif

#define DRIVER_NAME L"Hypervisor"

#ifdef _KERNEL_MODE
inline UNICODE_STRING g_deviceName = RTL_CONSTANT_STRING(L"\\Device\\" DRIVER_NAME);
inline UNICODE_STRING g_deviceSymbolicName = RTL_CONSTANT_STRING(L"\\??\\" DRIVER_NAME L"Link");
#endif

constexpr auto g_deviceNameUm = L"\\\\.\\" DRIVER_NAME L"Link";
