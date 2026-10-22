#include "VmExit.hpp"

#include "AsmHelpersC.hpp"
#include "CPU.hpp"
#include "Hypervisor.hpp"
#include "Logging.hpp"
#include "VMCS.hpp"
#include "Common/Utils.hpp"

using GuestRegs = Hypervisor::GuestRegs;

namespace VmExit {
    void PrepareVmxTermination(
        _Inout_ GuestRegs& Regs,
        _In_ const u64 RipAdvance
    ) {
        using enum VMCS::Field;
        
        const u64 rip = EXPECT(Cpu::Vmx::VmRead(GuestRip));
        
        if (EXPECT(VMCS::GetGuestCpl()) != VMCS::PrivilegeLevel::DplSystem) {
            KeBugCheckEx(static_cast<u32>(Hypervisor::BugCheck::DevirtualizeFromUserMode), rip, 0, 0, 0);
        }
        
        const u64 rsp = EXPECT(Cpu::Vmx::VmRead(GuestRsp));
        const u64 rflags = EXPECT(Cpu::Vmx::VmRead(GuestRflags));
        
        Cpu::Registers::Cr3 cr3{};
        cr3.All = EXPECT(Cpu::Vmx::VmRead(GuestCr3));
        Cpu::Registers::FsBase fsBase{};
        fsBase.All = EXPECT(Cpu::Vmx::VmRead(GuestFsBase));
        Cpu::Registers::Dr7 dr7{};
        dr7.All = EXPECT(Cpu::Vmx::VmRead(GuestDr7));
        Cpu::Registers::Ia32Debugctl debugctl{};
        debugctl.All = EXPECT(Cpu::Vmx::VmRead(GuestIa32Debugctl));
        
        cr3.Write();
        fsBase.Write();
        dr7.Write();
        debugctl.Write();
        
        const auto frame = bit_cast<u64*>(rsp - 2 * sizeof(u64));
        frame[0] = rflags;
        frame[1] = rip + RipAdvance;
        Regs.rsp = bit_cast<u64>(frame);
        
        g_hv.CurrentVcpu().OnTerminatedByExitHandler();
    }
    
    Expected<void>
    InjectHardwareException(
        _In_ const VMCS::ExceptionVector Vector
    ) {
        const VMCS::InterruptionInfo info{
            .Fields = {
                .vector = static_cast<u32>(Vector),
                .type = VMCS::InterruptionType::HardwareException,
                .deliverErrorCode = false,
                .valid = true
            },
        };
        return Cpu::Vmx::VmWrite(VMCS::Field::VmEntryIntrInfoField, info.All);
    }
    
    Expected<void>
    SetVmFailInvalid() {
        Cpu::Registers::Rflags rflags{};
        
        TRY_ASSIGN(rflags.All, Cpu::Vmx::VmRead(VMCS::Field::GuestRflags));
        
        rflags.Fields.carry = true;
        rflags.Fields.parity = false;
        rflags.Fields.auxiliaryCarry = false;
        rflags.Fields.zero = false;
        rflags.Fields.sign = false;
        rflags.Fields.overflow = false;
        
        TRY(Cpu::Vmx::VmWrite(VMCS::Field::GuestRflags, rflags.All));
        
        return Ok();
    }
    
    namespace Handler {
        Expected<HandlerResult>
        HandleCpuid(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(ExitQualification);
            
            TRY_ASSIGN(const bool terminate, Hypervisor::ShouldTerminateVmx(Regs));
            if (terminate) {
                return Ok(HandlerResult::TerminateRequested);
            }
            
            const u32 leaf = static_cast<u32>(Regs.rax);
            const u32 subleaf = static_cast<u32>(Regs.rcx);
            
            Cpu::Cpuid::Data cpuIdResult{};
            if (leaf == static_cast<u32>(Hypervisor::DetectMagic::Leaf)) {
                cpuIdResult.eax = static_cast<u32>(Hypervisor::DetectMagic::Signature);
            } else {
                cpuIdResult = Cpu::Cpuid::QueryEx(
                    static_cast<int>(leaf), 
                    static_cast<int>(subleaf)
                );
            }
            
            Regs.rax = cpuIdResult.eax;
            Regs.rbx = cpuIdResult.ebx;
            Regs.rcx = cpuIdResult.ecx;
            Regs.rdx = cpuIdResult.edx;
            
            return Ok(HandlerResult::Emulated);
        }
    
        Expected<HandlerResult>
        HandleCrAccess(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            using namespace VmExit::ExitQualification::CrAccess;
            const auto exitQualification = bit_cast<Fields>(ExitQualification);
            u64& operand = Regs[exitQualification.register_]; 
            
            if (exitQualification.register_ == Cpu::Gpr::Rsp) {
                TRY_ASSIGN(operand, Cpu::Vmx::VmRead(VMCS::Field::GuestRsp));
            }

            switch (exitQualification.accessType) {  // NOLINT(clang-diagnostic-switch-enum)
                case AccessType::MovToCr: {
                    switch (exitQualification.controlRegister) {
                        case ControlRegister::Cr0: {
                            TRY(Cpu::Vmx::VmWrite(VMCS::Field::GuestCr0, operand));
                            TRY(Cpu::Vmx::VmWrite(VMCS::Field::Cr0ReadShadow, operand));
                            break;
                        }
                        case ControlRegister::Cr3: {
                            TRY(Cpu::Vmx::VmWrite(VMCS::Field::GuestCr3, (operand & ~Cpu::Registers::Cr3::g_pcidNoInvalidate)));
                            break;
                        }
                        case ControlRegister::Cr4: {
                            TRY(Cpu::Vmx::VmWrite(VMCS::Field::GuestCr4, operand));
                            TRY(Cpu::Vmx::VmWrite(VMCS::Field::Cr4ReadShadow, operand));
                            break;
                        }
                        default: {
                            HvInfo("Unsupported register %u", static_cast<u32>(exitQualification.controlRegister));
                            break;
                        }
                    }  
                    break;
                }
                case AccessType::MovFromCr: {
                    switch (exitQualification.controlRegister) {
                        case ControlRegister::Cr0: {
                            TRY_ASSIGN(operand, Cpu::Vmx::VmRead(VMCS::Field::GuestCr0));
                            break;
                        }
                        case ControlRegister::Cr3: {
                            TRY_ASSIGN(operand, Cpu::Vmx::VmRead(VMCS::Field::GuestCr3));
                            break;
                        }
                        case ControlRegister::Cr4: {
                            TRY_ASSIGN(operand, Cpu::Vmx::VmRead(VMCS::Field::GuestCr4));
                            break;
                        }
                        default: {
                            HvInfo("Unsupported register %u", static_cast<u32>(exitQualification.controlRegister));
                            break;
                        }
                    }
                    break;
                }
                default: {
                    HvInfo("Unsupported operation %u", static_cast<u32>(exitQualification.accessType));
                    break;
                }
            }
            
            if (
                exitQualification.accessType == AccessType::MovFromCr &&
                exitQualification.register_ == Cpu::Gpr::Rsp
            ) {
                TRY(Cpu::Vmx::VmWrite(VMCS::Field::GuestRsp, operand));
            }
        
            return Ok(HandlerResult::Emulated);
        }
        
        Expected<HandlerResult>
        HandleMsrRead(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(ExitQualification);
            
            Cpu::Registers::MsrHalves msr{};
            
            const auto index = static_cast<u32>(Regs.rcx);
            msr.All = Msr::IsValid(index) ? Cpu::Msr::Read(index) : 0;
            
            Regs.rax = msr.Parts.low;
            Regs.rdx = msr.Parts.high;
            
            return Ok(HandlerResult::Emulated);
        }
        
        Expected<HandlerResult>
        HandleMsrWrite(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(ExitQualification);

            if (const auto index = static_cast<u32>(Regs.rcx); Msr::IsValid(index)) {
                Cpu::Registers::MsrHalves msr{};
                msr.Parts.low = static_cast<u32>(Regs.rax);
                msr.Parts.high = static_cast<u32>(Regs.rdx);
                Cpu::Msr::Write(index, msr.All);
            }
            
            return Ok(HandlerResult::Emulated);
        }
        
        Expected<HandlerResult>
        HandleVmxOperation(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(Regs);
            UNREFERENCED_PARAMETER(ExitQualification);
            
            return Ok(HandlerResult::FailVmxInstruction);
        }
        
        Expected<HandlerResult>
        HandleVmcall(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(Regs);
            UNREFERENCED_PARAMETER(ExitQualification);
            
            TRY_ASSIGN(const auto cpl, VMCS::GetGuestCpl());
            if (cpl != VMCS::PrivilegeLevel::DplSystem) {
                return Ok(HandlerResult::InjectInvalidOpcode);
            }
            return Ok(HandlerResult::ForwardHypercall);
        }
        
        Expected<HandlerResult>
        HandleXsetbv(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(ExitQualification);
            
            _xsetbv(
                static_cast<u32>(Regs.rcx),
                (Regs.rdx << 32) | static_cast<u32>(Regs.rax)
            );
            
            return Ok(HandlerResult::Emulated);
        }
        
        Expected<HandlerResult>
        HandleInvd(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(Regs);
            UNREFERENCED_PARAMETER(ExitQualification);
            
            __wbinvd();
            
            return Ok(HandlerResult::Emulated);
        }
        
        Expected<HandlerResult>
        HandleHlt(
            _Inout_ GuestRegs& Regs,
            _In_ const u64 ExitQualification
        ) {
            UNREFERENCED_PARAMETER(Regs);
            UNREFERENCED_PARAMETER(ExitQualification);
            
            return Ok(HandlerResult::Emulated);
        }
    }
    
    Expected<void>
    ResumeToNextInstruction() {
        using enum VMCS::Field;
        TRY_ASSIGN(const u64 currentRip, Cpu::Vmx::VmRead(GuestRip));
        TRY_ASSIGN(const u64 instructionLength, Cpu::Vmx::VmRead(VmExitInstructionLen));
        TRY(Cpu::Vmx::VmWrite(GuestRip, currentRip + instructionLength));
        
        TRY_ASSIGN(auto interruptibility, Cpu::Vmx::VmReadAs<VMCS::InterruptibilityState>(GuestInterruptibilityInfo));
        interruptibility.Fields.blockingBySti = false;
        interruptibility.Fields.blockingByMovSs = false;
        TRY(Cpu::Vmx::VmWrite(GuestInterruptibilityInfo, interruptibility.All));
        
        return Ok();
    }
    
    using ExitHandler = Expected<HandlerResult> (*)(
        _Inout_ Hypervisor::GuestRegs& Regs,
        _In_ u64 ExitQualification
    );
    
    constexpr u32 g_exitReasonCount = static_cast<u32>(VMCS::VmExitReason::Max);
    
    struct ExitHandlerTable {
        ExitHandler handlers[g_exitReasonCount]{};
        
        constexpr void Set(
            _In_ VMCS::VmExitReason Reason,
            _In_ const ExitHandler Handler
        ) {
            handlers[static_cast<u32>(Reason)] = Handler;
        }
        
        [[nodiscard]]
        constexpr ExitHandler Get(
            _In_ VMCS::VmExitReason Reason
        ) const {
            const auto index = static_cast<u32>(Reason);
            return index < g_exitReasonCount ? handlers[index] : nullptr;
        }
    };

    constexpr ExitHandlerTable g_exitHandlers = [] {
        using namespace VmExit::Handler;
        using enum VMCS::VmExitReason;
        
        ExitHandlerTable table{};
        table.Set(Cpuid, HandleCpuid);
        table.Set(CrAccess, HandleCrAccess);
        table.Set(MsrRead, HandleMsrRead);
        table.Set(MsrWrite, HandleMsrWrite);
        table.Set(Vmcall, HandleVmcall);
        table.Set(Xsetbv, HandleXsetbv);
        table.Set(Invd, HandleInvd);
        table.Set(Hlt, HandleHlt);

        constexpr VMCS::VmExitReason vmxInstructions[] = {
            Vmclear,
            Vmlaunch,
            Vmptrld,
            Vmptrst,
            Vmread,
            Vmresume,
            Vmwrite,
            Vmxoff,
            Vmxon,
        };
        
        for (const auto reason : vmxInstructions) {
            table.Set(reason, HandleVmxOperation);
        }
        
        return table;
    }();
    
    Expected<ExitAction>
    ApplyResult(
        _In_ const HandlerResult Result,
        _Inout_ GuestRegs& Regs
    ) {
        using enum HandlerResult;
        switch (Result) {
            case Emulated: {
                TRY(ResumeToNextInstruction());
                return Ok(ExitAction::Resume);
            }
            case ForwardHypercall: {
                TRY(ResumeToNextInstruction());
                return Ok(ExitAction::Hypercall);
            }
            case InjectInvalidOpcode: {
                TRY(InjectHardwareException(VMCS::ExceptionVector::InvalidOpcode));
                return Ok(ExitAction::Resume);
            }
            case FailVmxInstruction: {
                TRY(SetVmFailInvalid());
                TRY(ResumeToNextInstruction());
                return Ok(ExitAction::Resume);
            }
            case TerminateRequested: {
                HvInfo("Guest requested VMX termination");
                TRY_ASSIGN(const u64 len, Cpu::Vmx::VmRead(VMCS::Field::VmExitInstructionLen));
                PrepareVmxTermination(Regs, len);
                return Ok(ExitAction::Terminate);
            }
            case Unhandled: 
            default: {  // NOLINT(clang-diagnostic-covered-switch-default)
                HvFail("Shutting down VMX. Relevant RIP is 0x%llx", Cpu::Vmx::VmRead(VMCS::Field::GuestRip).ValueOr(0));
                PrepareVmxTermination(Regs, 0);
                return Ok(ExitAction::Terminate);
            }
        }
    }
    
    ExitAction
    CompleteExit(
        _In_ const HandlerResult Result,
        _Inout_ GuestRegs& Regs
    ) {
        const auto action = ApplyResult(Result, Regs);
        
        if ( action.IsOk()) {
            return action.Value();
        }
        
        HvFail("Could not apply exit result %u (status = 0x%x)", static_cast<u32>(Result), action.Status());
        PrepareVmxTermination(Regs, 0);
        return ExitAction::Terminate;
    }
    
    Expected<HandlerResult>
    DispatchExit(
        _Inout_ GuestRegs& Regs
    ) {
       using enum VMCS::Field;
        
        TRY_ASSIGN(const auto exitReason, Cpu::Vmx::VmReadAs<VMCS::ExitReason>(VmExitReason));
        const auto basicReason = exitReason.Fields.basicReason;
        
        if (exitReason.Fields.entryFailure) {
            HvFail("A previous VM entry failed, reason: 0x%lx", static_cast<u32>(exitReason.Fields.basicReason));
            return Ok(HandlerResult::Unhandled);
        }
        
        const auto handler = g_exitHandlers.Get(basicReason);
        if (handler == nullptr) {
            HvFail("Unhandled exit reason %llu", static_cast<u64>(basicReason));
            return Ok(HandlerResult::Unhandled);
        }
        
        TRY_ASSIGN(const u64 exitQualification, Cpu::Vmx::VmRead(ExitQualification));
        
        const auto result = handler(Regs, exitQualification);
        if (result.IsErr()) {
            HvFail(
                "Handler for exit reason %llu failed with error 0x%lx", 
                static_cast<u64>(basicReason),
                result.Status()
            );
        }
        
        return result;
    }
    
    extern "C"
    ExitAction
    MainVmexitHandler(
        _Inout_ GuestRegs& Regs
    ) {
        const auto result = DispatchExit(Regs);
        return CompleteExit(result.ValueOr(HandlerResult::Unhandled), Regs);
    }
}

extern "C" void VmResumeFailed(
    Hypervisor::GuestRegs& Regs
) {
    HvFail("Vmresume failed, error 0x%llx", Cpu::Vmx::VmLastError());
    VmExit::PrepareVmxTermination(Regs, 0);
}