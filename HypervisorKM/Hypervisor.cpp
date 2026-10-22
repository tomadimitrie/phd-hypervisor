#include "Hypervisor.hpp"

#include "Common/Utils.hpp"
#include "CPU.hpp"
#include "Memory.hpp"
#include "Logging.hpp"
#include "VMCS.hpp"
#include "AsmHelpersC.hpp"
#include "Lifetime.hpp"

constinit Hypervisor g_hv;

// ReSharper disable once CppInconsistentNaming
extern "C" NTSTATUS CLaunchOneCpu(
	void* GuestStack,
	Vcpu& Target
) {
	return Target.Launch(GuestStack).Status();
}

Expected<void> Hypervisor::Start() {
	ROLLBACK_DEFINE(rollback);
	
	TRY(Init());
	ERRDEFER(rollback) { Stop(); };
	
	TRY(Launch());
	
	return rollback.Ok();
}

Vcpu& Hypervisor::CurrentVcpu() const {
	const auto index = KeGetCurrentProcessorNumberEx(nullptr);
	NT_ASSERT(this->vcpus != nullptr && index < this->nrCpu);
	return this->vcpus[index];
}

_Use_decl_annotations_
Expected<void> Hypervisor::RunOnCpu(
	 const u32 Cpu,
	 const p_RunOnCpuHandler Handler
) const {
	PROCESSOR_NUMBER processorNumber{};
	
	TRY(Expected<void>::FromStatus(KeGetProcessorNumberFromIndex(Cpu, &processorNumber)));
	
	GROUP_AFFINITY affinity = {
		.Mask =  1ULL << processorNumber.Number,
		.Group = processorNumber.Group,
		.Reserved{},
	};
	GROUP_AFFINITY previousAffinity{};
	
	KeSetSystemGroupAffinityThread(&affinity, &previousAffinity);
	DEFER {
		KeRevertToUserGroupAffinityThread(&previousAffinity);
	};
	
	TRY(Handler(this->vcpus[Cpu]));
	
	return Ok();
}

_Use_decl_annotations_
Expected<void> Hypervisor::RunOnEveryCpu(
	 const p_RunOnCpuHandler Handler
) const {
	for (u32 i = 0; i < this->nrCpu; i += 1) {
		HvInfo("CPU %u start", i);
		TRY(RunOnCpu(i, Handler));
		HvInfo("CPU %u end", i);
	}
	
	return Ok();
}

Expected<void>
Hypervisor::Init() {
	ROLLBACK_DEFINE(rollback);
	
	HvInfo("Initializing hypervisor");
	
	this->nrCpu = KeQueryActiveProcessorCountEx(ALL_PROCESSOR_GROUPS);

	if (!Cpu::Vmx::CheckVmx()) {
		return Err(STATUS_NOT_SUPPORTED);
	}

	TRY_ASSIGN(this->vcpus, Memory::AllocatePoolArray<Vcpu>(this->nrCpu, false, Memory::PoolTag::GuestState));
	
	for (u32 i = 0; i < this->nrCpu; i += 1) {
		ConstructAt(&this->vcpus[i], i);
	}
	
	ERRDEFER(rollback) {
		Stop();
	};
	
	TRY(RunOnEveryCpu([](Vcpu& Target) {
		return Target.Init();
	}));
	HvSuccess("Hypervisor initialized");

	return rollback.Ok();
}

Expected<void>
Hypervisor::Launch() const {
	PAGED_CODE();

	HvInfo("Launching VM");
	
	return RunOnEveryCpu([](Vcpu& Target) {
		return Expected<void>::FromStatus(AsmLaunchOneCpu(Target));
	});
}

void
Hypervisor::Stop() {
	HvInfo("Destroying hypervisor");

	if (this->vcpus != nullptr) {
		(void)RunOnEveryCpu([](Vcpu& Target) -> Expected<void> {
			DestroyAt(&Target);
			return Ok();
		});
		
		Memory::FreePool(this->vcpus, Memory::PoolTag::GuestState);
		this->vcpus = nullptr;
	}

	HvSuccess("Hypervisor destroyed");
}

_Use_decl_annotations_
Expected<bool> 
Hypervisor::ShouldTerminateVmx(
	const GuestRegs& Regs
) {
	const bool magicMatches = 
		Regs.rax == static_cast<u32>(TerminateMagic::Rax) &&
		Regs.rcx == static_cast<u32>(TerminateMagic::Rcx);
	if (!magicMatches) {
		return Ok(false);
	}
	TRY_ASSIGN(const auto cpl, VMCS::GetGuestCpl());
	return Ok(cpl == VMCS::PrivilegeLevel::DplSystem);
}

void Hypervisor::RequestVmxTermination() {
	Cpu::Cpuid::QueryEx(
		static_cast<u32>(TerminateMagic::Rax),
		static_cast<u32>(TerminateMagic::Rcx)
	);
}

bool Hypervisor::IsInHypervisor() {
	return Cpu::Cpuid::QueryEx(
		static_cast<int>(DetectMagic::Leaf), 
		0
	).eax == static_cast<u32>(DetectMagic::Signature);
}
