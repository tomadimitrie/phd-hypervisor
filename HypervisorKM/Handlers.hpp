#pragma once

#include <ntddk.h>

NTSTATUS 
OnCreate(
	_In_ PDEVICE_OBJECT DeviceObject, 
	_Inout_ PIRP Irp
);

NTSTATUS 
OnClose(
	_In_ PDEVICE_OBJECT DeviceObject, 
	_Inout_ PIRP Irp
);

NTSTATUS 
OnIoctl(
	_In_ PDEVICE_OBJECT DeviceObject, 
	_Inout_ PIRP Irp
);
