#pragma once

#include "CPU.hpp"
#include "Common/Utils.hpp"

namespace VmExit {
    namespace Msr {
        constexpr Range<u32> g_lowRange{
            .first = 0x00000000,
            .last = 0x00001fff
        };
        constexpr Range<u32> g_highRange{
            .first = 0xc0000000,
            .last = 0xc0001fff,
        };
        constexpr Range<u32> g_reservedRange{
            .first = 0x40000000,
            .last = 0x400000f0,
        };
        
        [[nodiscard]]
        constexpr bool IsValid(
            _In_ const u32 Index
        ) {
            return g_lowRange.Contains(Index) ||
                g_highRange.Contains(Index) ||
                g_reservedRange.Contains(Index);
        }
    }

    namespace ExitQualification {
        namespace CrAccess {
            enum class ControlRegister : u32 {
                Cr0 = 0,
                Cr3 = 3,
                Cr4 = 4,
                Cr8 = 8,
            };
            enum class AccessType : u32 {
                MovToCr = 0,
                MovFromCr = 1,
                Clts = 2,
                Lmsw = 3,
            };
            enum class LmswOperandType : u32 {
                Register = 0,
                Memory = 1,
            };
            struct Fields {
                ControlRegister controlRegister : 4;
                AccessType accessType : 2;
                u32 lmswOperandType : 1;
                u32 reserved0 : 1 = 0;
                Cpu::Gpr register_ : 4;
                u32 reserved1 : 4 = 0;
                u32 lmswSourceData : 16;
                u32 reserved2 : 32;
            };
            static_assert(sizeof(Fields) == sizeof(u64));
        }
    }
    
    enum class HandlerResult : u8 {
        Emulated,
        ForwardHypercall,
        InjectInvalidOpcode,
        FailVmxInstruction,
        TerminateRequested,
        Unhandled,
    };
    
    enum class ExitAction : u8 {
        Resume = 0,
        Terminate = 1,
        Hypercall = 2,
    };
    
}