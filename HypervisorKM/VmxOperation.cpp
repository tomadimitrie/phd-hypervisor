#include "VmxOperation.hpp"

#include "Hypervisor.hpp"


_Use_decl_annotations_
Expected<void> VmxOperation::Enter(
    const Cpu::Vmx::VmxRegion& VmxonRegion
) {
    NT_ASSERT(!vmxEnabled && !inVmxOperation);
    
    Cpu::Vmx::EnableVmxInFeatureControl();
    Cpu::Vmx::WithFixedBits(Cpu::Registers::Cr0::Read()).Write();
    
    auto cr4 = Cpu::Registers::Cr4::Read();
    cr4.Fields.vmxe = true;
    Cpu::Vmx::WithFixedBits(cr4).Write();
    vmxEnabled = true;
    
    TRY(Cpu::Vmx::VmxOn(VmxonRegion));
    inVmxOperation = true;
    
    return Ok();
}

void VmxOperation::Leave() {
    if (inVmxOperation) {
        if (Hypervisor::IsInHypervisor()) {
            Hypervisor::RequestVmxTermination();
        } else {
            Cpu::Vmx::VmxOff();
        }
        inVmxOperation = false;
    }
    DisableVmx();
}

void VmxOperation::DisableVmx() {
    if (!vmxEnabled) {
        return;
    }
    
    auto cr4 = Cpu::Registers::Cr4::Read();
    cr4.Fields.vmxe = false;
    cr4.Write();
    vmxEnabled = false;
}
