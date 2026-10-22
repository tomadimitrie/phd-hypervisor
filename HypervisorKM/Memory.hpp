#pragma once

#include <sal.h>

#include "Common/Utils.hpp"

namespace Memory {
	enum class PoolTag : u32 {
		None = 0,
		GuestState = 'TSG@', // @GST
		Eptp = 'PPE@', // @EPP
		EptPml4E = '4MP@', // @PM4
		EptPdpte = 'TPD@', // @DPT
		EptPde = 'EDP@', // @PDE
		EptPte = 'ETP@', // @PTE
		Rip = 'PIR@', // @RIP
		Rsp = 'PSR@', // @RSP
		Msr = 'RSM@', // @MSR
		GuestMemory = 'EMG@' // @GME
	};

	enum class Type : u8 {
		Hpa,
		Hva,
	};

	enum class Allocator : u8 {
		NonPagedPool,
		PagedPool,
		Contiguous,
	};
	
	template <typename T>
	Expected<T*> AllocatePoolArray(
		_In_ const u64 Count,
		_In_ const bool Paged,
		_In_ const PoolTag Tag
	) {
		return Expected<T*>::FromPointer(static_cast<T*>(ExAllocatePool2(
			Paged ? POOL_FLAG_PAGED : POOL_FLAG_NON_PAGED,
			sizeof(T) * Count,
			static_cast<u32>(Tag)
		)));
	}

	inline void FreePool(
		_In_ void* Pointer,
		_In_ const PoolTag Tag
	) {
		ExFreePoolWithTag(Pointer, static_cast<u32>(Tag));
	}

	template <Type>
	struct AddressDetails {};

	template <>
	struct AddressDetails<Type::Hpa> {
		void* originalVa = nullptr;
	};

	template <Type TType = Type::Hva>
	struct Address : AddressDetails<TType> {
		u64 inner = 0;

		constexpr Address() noexcept = default;
		// ReSharper disable once CppNonExplicitConvertingConstructor
		constexpr Address(const u64 Ptr) noexcept : inner(Ptr) {}
		// ReSharper disable once CppNonExplicitConvertingConstructor
		constexpr Address(void* Ptr) noexcept : inner(bit_cast<u64>(Ptr)) {}

		u64& Val() {
			return inner;
		}

		[[nodiscard]] void* Vptr() const {
			return bit_cast<void*>(inner);
		}

		[[nodiscard]] u8* Ptr() const {
			return bit_cast<u8*>(inner);
		}

		[[nodiscard]] 
		Address<Type::Hpa> ToHpa() const noexcept
			requires (TType == Type::Hva)
		{
			const auto pAddr = MmGetPhysicalAddress(Vptr());
			Address<Type::Hpa> result(static_cast<u64>(pAddr.QuadPart));
			result.originalVa = Vptr();
			return result;
		}

		[[nodiscard]]
		Address<Type::Hva> ToHva() const noexcept
			requires (TType == Type::Hpa)
		{
			if (this->originalVa != nullptr) {
				return this->originalVa;
			}

			return MmGetVirtualForPhysical({
				.QuadPart = static_cast<LONGLONG>(inner),
			});
		}
	};

	template <Type TType>
	using Unowned = Address<TType>;

	template <
		Allocator TAllocator,
		u64 TSize,
		PoolTag TTag = PoolTag::None
	>
	class Allocation {
		static constexpr bool g_isPool = TAllocator == Allocator::NonPagedPool || TAllocator == Allocator::PagedPool;
		
		static_assert(TSize > 0, "Size must be greater than 0");
		static_assert(g_isPool == (TTag != PoolTag::None), "Tag required for pool allocations, or no tag otherwise");
		
	public:
		Allocation() = default;
		
		~Allocation() {
			Free();
		}
		
		Allocation(const Allocation&) = delete;
		Allocation& operator=(const Allocation&) = delete;
		
		Allocation(Allocation&&) = delete;
		Allocation& operator=(Allocation&&) = delete;
		
		Expected<void> Allocate() {
			NT_ASSERT(virtualAddress == nullptr);
			
			if constexpr (TAllocator == Allocator::NonPagedPool) {
				virtualAddress = ExAllocatePool2(POOL_FLAG_NON_PAGED, TSize, static_cast<u32>(TTag));
			} else if constexpr (TAllocator == Allocator::PagedPool) {
				virtualAddress = ExAllocatePool2(POOL_FLAG_PAGED, TSize, static_cast<u32>(TTag));
			} else if constexpr (TAllocator == Allocator::Contiguous) {
				virtualAddress = MmAllocateContiguousMemory(TSize, { .QuadPart = MAXLONGLONG });
			}
			
			if (virtualAddress == nullptr) {
				return Err(STATUS_INSUFFICIENT_RESOURCES);
			}
			
			RtlSecureZeroMemory(virtualAddress, TSize);
			
			return Ok();
		}
		
		[[nodiscard]] void* Virtual() const {
			return virtualAddress;
		}
		
		[[nodiscard]] u64 Physical() const 
			requires (
				TAllocator == Allocator::Contiguous ||
				(TAllocator == Allocator::NonPagedPool && TSize <= PAGE_SIZE)
			)
		{
			NT_ASSERT(virtualAddress != nullptr);
			return static_cast<u64>(MmGetPhysicalAddress(virtualAddress).QuadPart);
		}
		
		[[nodiscard]] static constexpr u64 Size() {
			return TSize;
		}
		
	private:
		void Free() {
			if (virtualAddress == nullptr) {
				return;
			}	
			
			if constexpr (g_isPool) {
				ExFreePoolWithTag(virtualAddress, static_cast<u32>(TTag));
			} else {
				MmFreeContiguousMemory(virtualAddress);
			}
			
			virtualAddress = nullptr;
		}
		
		void* virtualAddress = nullptr;
	};
	
	template <PoolTag TTag, u64 TSize>
	using PagedPoolAllocation = Allocation<Allocator::PagedPool, TSize, TTag>;
	
	template <PoolTag TTag, u64 TSize>
	using NonPagedPoolAllocation = Allocation<Allocator::NonPagedPool, TSize, TTag>;
	
	template <u64 TSize>
	using ContiguousAllocation = Allocation<Allocator::Contiguous, TSize>;
}
