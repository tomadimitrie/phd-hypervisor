#pragma once

#include "CPU.hpp"
#include "EPT.hpp"
#include "VmxOperation.hpp"
#include "Common/Utils.hpp"

class Vcpu {
public:
    static constexpr u64 g_stackSize = 0x8000;
    
    explicit Vcpu(const u32 CpuIndex) : cpuIndex(CpuIndex) {}
    
    ~Vcpu() {
        NT_ASSERT(IsOnOwnCpu());
    }
    
    Vcpu(const Vcpu&) = delete;
    Vcpu& operator=(const Vcpu&) = delete;
    Vcpu(Vcpu&&) = delete;
    Vcpu& operator=(Vcpu&&) = delete;
    
    [[nodiscard]]
    u32 CpuIndex() const {
        return cpuIndex;
    }
    
    Expected<void> Init();
    
    Expected<void> Launch(
        _In_ void* GuestStack
    );
    
    void OnTerminatedByExitHandler() {
        vmx.OnTerminatedByExitHandler();
    }
    
private:
    [[nodiscard]]
    bool IsOnOwnCpu() const {
        return KeGetCurrentProcessorNumberEx(nullptr) == cpuIndex;
    }
    
    Expected<void> SetupVmcs(
        _In_ void* GuestStack
    );
    
    u32 cpuIndex;
    
    Memory::NonPagedPoolAllocation<Memory::PoolTag::Rsp, g_stackSize> stack;
    Cpu::Vmx::VmxRegion vmxonRegion;
    Cpu::Vmx::VmxRegion vmcsRegion;
    Memory::NonPagedPoolAllocation<Memory::PoolTag::Eptp, PAGE_SIZE> eptp;
    Memory::NonPagedPoolAllocation<Memory::PoolTag::Msr, PAGE_SIZE> msrBitmap;
		
    Ept::EptState eptState{};
		
    VmxOperation vmx;
};