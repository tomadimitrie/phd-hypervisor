#include "Handlers.hpp"
#include "Common/Driver.hpp"
#include "Common/Utils.hpp"
#include "Common/Runtime/CRT.hpp"
#include "Logging.hpp"

namespace {
	void
	DriverUnload(
		// ReSharper disable once CppParameterMayBeConstPtrOrRef incompatible signature
		_In_ DRIVER_OBJECT* DriverObject
	) {
		(void)IoDeleteSymbolicLink(&g_deviceSymbolicName);
		IoDeleteDevice(DriverObject->DeviceObject);
		Crt::Uninit();
	}
	
	Expected<void> InitializeDriver(
		_In_ const PDRIVER_OBJECT DriverObject
	) {
		ROLLBACK_DEFINE(rollback);
		
		TRY(Crt::Init());
		ERRDEFER(rollback) {
			Crt::Uninit();
		};
		
		DriverObject->DriverUnload = DriverUnload;
		DriverObject->MajorFunction[IRP_MJ_CREATE] = OnCreate;
		DriverObject->MajorFunction[IRP_MJ_CLOSE] = OnClose;
		DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = OnIoctl;
		
		PDEVICE_OBJECT device = nullptr;
		TRY(Expected<void>::FromStatus(IoCreateDevice(
			DriverObject, 
			0, 
			&g_deviceName,
			FILE_DEVICE_UNKNOWN, 
			FILE_DEVICE_SECURE_OPEN,
			true,
			&device
		)));
		ERRDEFER(rollback) {
			IoDeleteDevice(device);
		};
		
		TRY(Expected<void>::FromStatus(
			IoCreateSymbolicLink(
				&g_deviceSymbolicName,
				&g_deviceName
			)
		));
	
		device->Flags &= ~DO_DEVICE_INITIALIZING;
		
		HvSuccess("Driver loaded");
		return rollback.Ok();
	}
}

extern "C"
NTSTATUS
DriverEntry(
	_In_ const PDRIVER_OBJECT DriverObject,
	_In_ const PUNICODE_STRING RegistryPath
) {
	UNREFERENCED_PARAMETER(RegistryPath);

	return InitializeDriver(DriverObject).Status();
}