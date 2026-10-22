#pragma once

#include "CPU.hpp"
#include "Vcpu.hpp"

class Hypervisor {
public:
	struct GuestRegs {
		u64 rax;
		u64 rcx;
		u64 rdx;
		u64 rbx;
		u64 rsp;
		u64 rbp;
		u64 rsi;
		u64 rdi;
		u64 r8;
		u64 r9;
		u64 r10;
		u64 r11;
		u64 r12;
		u64 r13;
		u64 r14;
		u64 r15;
		
		[[nodiscard]] u64& operator[](const Cpu::Gpr Register) {
			NT_ASSERT(Register < Cpu::Gpr::Count);
			return this->*g_byEncoding[static_cast<u32>(Register)];
		}
		
		static constexpr u64 GuestRegs::* g_byEncoding[] = {
			&GuestRegs::rax,
			&GuestRegs::rcx,
			&GuestRegs::rdx,
			&GuestRegs::rbx,
			&GuestRegs::rsp,
			&GuestRegs::rbp,
			&GuestRegs::rsi,
			&GuestRegs::rdi,
			&GuestRegs::r8,
			&GuestRegs::r9,
			&GuestRegs::r10,
			&GuestRegs::r11,
			&GuestRegs::r12,
			&GuestRegs::r13,
			&GuestRegs::r14,
			&GuestRegs::r15,
		};
		static_assert(sizeof(g_byEncoding) / sizeof(g_byEncoding[0]) == static_cast<u32>(Cpu::Gpr::Count));
	};
	static_assert(sizeof(GuestRegs) == 16 * sizeof(u64));
	static_assert(FIELD_OFFSET(GuestRegs, rsp) == 4 * sizeof(u64));
	
	enum class BugCheck : u32 {
		VmResumeFailed = 'FRMV', // VMRF
		DevirtualizeFromUserMode = 'MUDV', // DVUM
	};
	
	enum class TerminateMagic : u32 {
		Rax = 0x41414141,
		Rcx = 0x42424242,
	};
	
	enum class DetectMagic : u32 {
		Leaf = 'HVHV',
		Signature = 'HVOK',
	};
	
	Expected<void> Start();
	
	void Stop();
	
	[[nodiscard]] Vcpu& CurrentVcpu() const;
	
	static
	Expected<bool>
	ShouldTerminateVmx(
		_In_ const GuestRegs& Regs
	);
	
	static
	void 
	RequestVmxTermination();
	
	static
	bool
	IsInHypervisor();
	
private:	
	using p_RunOnCpuHandler = Expected<void> (*)(_In_ Vcpu& Target);
	
	Expected<void> RunOnCpu(
		_In_ u32 Cpu,
		_In_ p_RunOnCpuHandler Handler
	) const;
	
	Expected<void> RunOnEveryCpu(
		_In_ p_RunOnCpuHandler Handler
	) const;

	Expected<void> Init();
	
	Expected<void> Launch() const;
	
	u32 nrCpu = 0; 
	Vcpu* vcpus = nullptr;
};

extern Hypervisor g_hv;

