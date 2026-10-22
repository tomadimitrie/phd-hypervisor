#include "EPT.hpp"

#include "CPU.hpp"
#include "Logging.hpp"
#include "Memory.hpp"

namespace Ept {
	NTSTATUS Eptp::Init(void*& Addr) {
		PAGED_CODE();
		Addr;
		
		NTSTATUS status = STATUS_UNSUCCESSFUL;
	// 	Eptp* eptp = nullptr;
	// 	Pml4E* pml4E = nullptr;
	// 	Pdpte* pdpte = nullptr; 
	// 	Pde* pde = nullptr;
	// 	Pte* pte = nullptr;
	// 	void* guestMemory = nullptr;
	// 	constexpr u64 pagesToAllocate = 100;
	//
	// 	BAIL_IF_NULL_ASSIGN_STATUS(eptp, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED,
	// 		PAGE_SIZE,
	// 		POOL_TAG::Eptp
	// 	);
	// 	RtlSecureZeroMemory(eptp, PAGE_SIZE);
	//
	// 	BAIL_IF_NULL_ASSIGN_STATUS(pml4E, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED,
	// 		PAGE_SIZE,
	// 		POOL_TAG::EptPml4E
	// 	);
	// 	RtlSecureZeroMemory(pml4E, PAGE_SIZE);
	//
	// 	BAIL_IF_NULL_ASSIGN_STATUS(pdpte, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED,
	// 		PAGE_SIZE,
	// 		POOL_TAG::EptPdpte
	// 	);
	// 	RtlSecureZeroMemory(pdpte, PAGE_SIZE);
	//
	// 	BAIL_IF_NULL_ASSIGN_STATUS(pde, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED,
	// 		PAGE_SIZE,
	// 		POOL_TAG::EptPde
	// 	);
	// 	RtlSecureZeroMemory(pde, PAGE_SIZE);
	//
	// 	BAIL_IF_NULL_ASSIGN_STATUS(pte, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED,
	// 		PAGE_SIZE,
	// 		POOL_TAG::EptPte
	// 	);
	// 	RtlSecureZeroMemory(pte, PAGE_SIZE);
	// 	
	// 	BAIL_IF_NULL_ASSIGN_STATUS(guestMemory, STATUS_INSUFFICIENT_RESOURCES, ExAllocatePool2,
	// 		POOL_FLAG_NON_PAGED_EXECUTE,
	// 		PAGE_SIZE * pagesToAllocate,
	// 		POOL_TAG::GuestMemory
	// 	);
	// 	RtlFillMemory(guestMemory, PAGE_SIZE * pagesToAllocate, 0xF4);
	//
	// 	for (u64 i = 0; i < pagesToAllocate; i += 1) {
	// 		pte[i].Fields = {
	// 			.read = true,
	// 			.write = true,
	// 			.execute = 1,
	// 			.physicalAddress = Memory::Unowned<Memory::Type::Hva>(static_cast<u8*>(guestMemory) + i * PAGE_SIZE).ToHpa().Val() >> 12,
	// 		};
	// 	}
	//
	// 	pde->Fields = {
	// 		.read = true,
	// 		.write = true,
	// 		.execute = true,
	// 		.physicalAddress = Memory::Unowned<Memory::Type::Hva>(pte).ToHpa().Val() >> 12,
	// 	};
	//
	// 	pdpte->Fields = {
	// 		.read = true,
	// 		.write = true,
	// 		.execute = true,
	// 		.physicalAddress = Memory::Unowned<Memory::Type::Hva>(pde).ToHpa().Val() >> 12,
	// 	};
	//
	// 	pml4E->Fields = {
	// 		.read = true,
	// 		.write = true,
	// 		.execute = true,
	// 		.physicalAddress = Memory::Unowned<Memory::Type::Hva>(pdpte).ToHpa().Val() >> 12,
	// 	};
	//
	// 	eptp->Fields = {
	// 		.pml4Address = Memory::Unowned<Memory::Type::Hva>(pml4E).ToHpa().Val() >> 12,
	// 	};
	//
	// 	HvInfo("EPTP at %p", eptp);
	//
	// 	Addr = eptp;
	// 	// g_guestMemory = guestMemory;
	// 	status = STATUS_SUCCESS;
	// end:
	// 	if (!NT_SUCCESS(status)) {
	// 		if (guestMemory != nullptr) {
	// 			ExFreePoolWithTag(guestMemory, POOL_TAG::GuestMemory);
	// 		}
	// 		if (pte != nullptr) {
	// 			ExFreePoolWithTag(pte, POOL_TAG::EptPte);
	// 		}
	// 		if (pde != nullptr) {
	// 			ExFreePoolWithTag(pde, POOL_TAG::EptPde);
	// 		}
	// 		if (pdpte != nullptr) {
	// 			ExFreePoolWithTag(pdpte, POOL_TAG::EptPdpte);
	// 		}
	// 		if (pml4E != nullptr) {
	// 			ExFreePoolWithTag(pml4E, POOL_TAG::EptPml4E);
	// 		}
	// 		if (eptp != nullptr) {
	// 			ExFreePoolWithTag(eptp, POOL_TAG::Eptp);
	// 		}
	// 	}
	//
		return status;
	}

	bool CheckFeatures() {
		const auto mtrr = Cpu::Registers::Ia32MtrrDefType::Read();
		if (!mtrr.Fields.mtrrEnable) {
			HvFail("Mtrr Dynamic Ranges not supported");
			return false;
		}
		return true;
	}
	
	_Use_decl_annotations_
	NTSTATUS
	BuildMtrrMap(
		u32& NumberOfEnabledMemoryRanges,
		MtrrRange* MemoryRanges,
		const u32 MaxMemoryRanges
	) {
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		
		NumberOfEnabledMemoryRanges = 0;
		__try {
			const auto mtrrCapabilities = Cpu::Registers::Ia32MtrrCapabilities::Read();
			if (mtrrCapabilities.Fields.variableRangeCount > MaxMemoryRanges) {
				status = STATUS_BUFFER_OVERFLOW;
				__leave;
			}
			
			const auto mtrrDefType = Cpu::Registers::Ia32MtrrDefType::Read(); 
		
			for (u32 i = 0; i < mtrrCapabilities.Fields.variableRangeCount; i += 1) {
				const auto base = Cpu::Registers::Ia32MtrrPhysBase::Read(i);
				const auto mask = Cpu::Registers::Ia32MtrrPhysMask::Read(i);
				ULONG lowestMaskBit = 0;
			
				if (!mask.Fields.valid) {
					continue;
				}

				if (base.Fields.type == mtrrDefType.Fields.defaultMemoryType) {
					continue;
				}
				
				if (!_BitScanForward64(&lowestMaskBit, mask.Fields.pageFrameNumber << PAGE_SHIFT)) {
					continue;
				}
				const auto rangeBase = base.Fields.pageFrameNumber << PAGE_SHIFT;
				const auto rangeSize = 1ULL << lowestMaskBit;
				MemoryRanges[NumberOfEnabledMemoryRanges] = {
					.physicalBase = rangeBase,
					.physicalEnd = rangeBase + rangeSize - 1,
					.memoryType = static_cast<MemoryType>(base.Fields.type),
				};
				NumberOfEnabledMemoryRanges += 1;
			}
		
			status = STATUS_SUCCESS;
		} __finally {}
		return status;
	}
}
