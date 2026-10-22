#pragma once

#include "CPU.hpp"
#include "Common/Utils.hpp"

class VmxOperation {
public:
    VmxOperation() = default;
    
    ~VmxOperation() {
        Leave();
    }
    
    VmxOperation(const VmxOperation&) = delete;
    VmxOperation& operator=(const VmxOperation&) = delete;
    VmxOperation(VmxOperation&&) = delete;
    VmxOperation& operator=(VmxOperation&&) = delete;
    
    Expected<void> Enter(
        _In_ const Cpu::Vmx::VmxRegion& VmxonRegion
    );
    
    void OnTerminatedByExitHandler() {
        inVmxOperation = false;
    }
private:
    void Leave();
    void DisableVmx();
    bool vmxEnabled = false;
    bool inVmxOperation = false;
};