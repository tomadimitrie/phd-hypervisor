#pragma once

#include "Common/Utils.hpp"

namespace Ept {
	union Eptp {
		u64 All;
		struct {
			u64 memoryType : 3 = 6;
			u64 pageWalkLength : 3 = 3;
			u64 dirtyAndAccessedEnabled : 1 = 1;
			u64 reserved0 : 5 = 0;
			u64 pml4Address : 36;
			u64 reserved1 : 16 = 0;
		} Fields;

		static NTSTATUS Init(void*& Addr);
	};
	static_assert(sizeof(Eptp) == sizeof(u64));

	union Pml4E {
		u64 All;
		struct {
			u64 read : 1;
			u64 write : 1;
			u64 execute : 1;
			u64 reserved0 : 5 = 0;
			u64 accessed : 1 = 0;
			u64 ignored0 : 1 = 0;
			u64 executeForUser : 1 = 0;
			u64 ignored1 : 1 = 0;
			u64 physicalAddress : 36;
			u64 reserved1 : 4 = 0;
			u64 ignored2 : 12 = 0;
		} Fields;
	};
	static_assert(sizeof(Pml4E) == sizeof(u64));

	union Pdpte {
		u64 All;
		struct {
			u64 read : 1;
			u64 write : 1;
			u64 execute : 1;
			u64 reserved0 : 5 = 0;
			u64 accessed : 1 = 0;
			u64 ignored0 : 1 = 0;
			u64 executeForUser : 1 = 0;
			u64 ignored1 : 1 = 0;
			u64 physicalAddress : 36;
			u64 reserved1 : 4 = 0;
			u64 ignored2 : 12 = 0;
		} Fields;
	};
	static_assert(sizeof(Pdpte) == sizeof(u64));

	union Pde {
		u64 All;
		struct {
			u64 read : 1;
			u64 write : 1;
			u64 execute : 1;
			u64 reserved0 : 5 = 0;
			u64 accessed : 1 = 0;
			u64 ignored0 : 1 = 0;
			u64 executeForUser : 1 = 0;
			u64 ignored1 : 1 = 0;
			u64 physicalAddress : 36;
			u64 reserved1 : 4 = 0;
			u64 ignored2 : 12 = 0;
		} Fields;
	};
	static_assert(sizeof(Pde) == sizeof(u64));

	union Pte {
		u64 All;
		struct {
			u64 read : 1;
			u64 write : 1;
			u64 execute : 1;
			u64 eptMemoryType : 3 = 6;
			u64 ignorePat : 1 = 0;
			u64 ignored0 : 1 = 0;
			u64 accessed : 1 = 0;
			u64 dirty : 1 = 0;
			u64 executeForUser : 1 = 0;
			u64 ignored1 : 1 = 0;
			u64 physicalAddress : 36;
			u64 reserved1 : 4 = 0;
			u64 ignored2 : 11 = 0;
			u64 suppressVe : 1 = 0;
		} Fields;
	};
	static_assert(sizeof(Pte) == sizeof(u64));
	
	enum MemoryType : u8 {
		Uncacheable = 0,
		WriteCombining = 1,
		WriteThrough = 4,
		WriteProtected = 5,
		WriteBack = 6,
		Invalid = 0xff,
	};
	
	struct MtrrRange {
		u64 physicalBase;
		u64 physicalEnd;
		MemoryType memoryType;
	};
	
	struct EptState {
		u32 numberOfEnabledMemoryRanges;
		MtrrRange memoryRanges[10];
	};
	
	bool CheckFeatures();
	
	NTSTATUS
	BuildMtrrMap(
		_Out_ u32& NumberOfEnabledMemoryRanges,
		_Inout_ MtrrRange* MemoryRanges,
		_In_ u32 MaxMemoryRanges
	);
	
}
