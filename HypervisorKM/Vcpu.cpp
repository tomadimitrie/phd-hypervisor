#include "Vcpu.hpp"

Expected<void> Vcpu::Init() {
    const auto vmxBasic = Cpu::Registers::Ia32VmxBasic::Read();
    const auto revision = static_cast<u32>(vmxBasic.Fields.revision);

    HvInfo("Allocating Vmxon region");
    TRY(vmxonRegion.Allocate());
    *static_cast<u32*>(vmxonRegion.Virtual()) = revision;
    TRY(vmx.Enter(vmxonRegion));

    HvInfo("Allocating VMCS region");
    TRY(vmcsRegion.Allocate());
    *static_cast<u32*>(vmcsRegion.Virtual()) = revision;
    TRY(Cpu::Vmx::VmPtrLd(vmcsRegion));
	
    return Ok();
}

Expected<void> Vcpu::Launch(
    _In_ void* GuestStack
) {
    TRY(stack.Allocate());
    TRY(msrBitmap.Allocate());

    TRY(Cpu::Vmx::VmClear(vmcsRegion));
    TRY(Cpu::Vmx::VmPtrLd(vmcsRegion));

    HvInfo("Setup VMCS");

    TRY(SetupVmcs(GuestStack));

    const Err failure = Cpu::Vmx::VmLaunch();
    HvFail("VmLaunch failed, error %llu", Cpu::Vmx::VmLastError());

    return failure;
}
