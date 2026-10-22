#include "Handlers.hpp"

#include "Common/Utils.hpp"
#include "CPU.hpp"
#include "Logging.hpp"
#include "Hypervisor.hpp"

namespace {
	NTSTATUS CompleteRequest(
		_Inout_ const PIRP Irp,
		_In_ const NTSTATUS Status,
		_In_ const ULONG_PTR Information = 0
	) {
		Irp->IoStatus.Status = Status;
		Irp->IoStatus.Information = Information;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return Status;
	}
	
	Expected<void> HandleIoctl(
		_In_ const u32 ControlCode
	) {
		UNREFERENCED_PARAMETER(ControlCode);
		return Err(STATUS_NOT_SUPPORTED);
		// switch (ControlCode) {
		// 	default: {
		// 		return Err(STATUS_NOT_SUPPORTED);
		// 	}
		// }
	}
}

NTSTATUS
OnCreate(
	_In_ const PDEVICE_OBJECT DeviceObject,
	_Inout_ const PIRP Irp
) {
	UNREFERENCED_PARAMETER(DeviceObject);

	return CompleteRequest(Irp, g_hv.Start().Status());
}

NTSTATUS 
OnClose(
	_In_ const PDEVICE_OBJECT DeviceObject,
	_Inout_ const PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	g_hv.Stop();

	return CompleteRequest(Irp, STATUS_SUCCESS);
}

NTSTATUS
OnIoctl(
	_In_ const PDEVICE_OBJECT DeviceObject,
	_Inout_ const PIRP Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	
	const auto stack = IoGetCurrentIrpStackLocation(Irp);
	const auto result = HandleIoctl(stack->Parameters.DeviceIoControl.IoControlCode);
	
	return CompleteRequest(Irp, result.Status());
}
