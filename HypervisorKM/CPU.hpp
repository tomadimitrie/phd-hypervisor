#pragma once

#include <sal.h>
#include <intrin.h>

#include "Logging.hpp"
#include "Common/Utils.hpp"
#include "Memory.hpp"
#include "VMCS.hpp"

namespace Cpu {
	enum class Gpr : u32 {
		Rax = 0,
		Rcx = 1,
		Rdx = 2,
		Rbx = 3,
		Rsp = 4,
		Rbp = 5,
		Rsi = 6,
		Rdi = 7,
		R8 = 8,
		R9 = 9,
		R10 = 10,
		R11 = 11,
		R12 = 12,
		R13 = 13,
		R14 = 14,
		R15 = 15,
		
		Count
	};
	
	namespace Cpuid {
		struct Data {
			u32 eax;
			u32 ebx;
			u32 ecx;
			u32 edx;
		};

		Data Query(
			_In_ int Leaf
		);
		
		Data QueryEx(
			_In_ int Leaf,
			_In_ int Subleaf
		);

		void GetCpuVendor(
			_In_ unsigned char Vendor[]
		);
	}

	namespace Registers {
		template <typename TDerived, typename TValue, typename... TArgs>
		concept RawAccessible = requires(TValue Value, TArgs... Args) {
			requires g_isSame<decltype(TDerived::ReadRaw(Args...)), TValue>;
			requires g_isSame<decltype(TDerived::WriteRaw(Value, Args...)), void>;
		};
		
		enum class MsrAddress : u32;
		
		template <typename TDerived, typename TBits, MsrAddress TAddress>
		struct Msr;	
		
		template <typename TDerived, typename TBits, MsrAddress TFirst, u32 TStride>
		struct IndexedMsr;
		
		template <typename TDerived, typename TBits, u32 TIndex>
		struct Dr;

		template <typename TDerived, typename TBits, typename TValue>
		struct RegisterBase {
		private:
			RegisterBase() = default;

		public:
			static_assert(sizeof(TBits) == sizeof(TValue), "Bitfield view must match register width");

			union {
				TValue All;
				TBits Fields;
			};

			template <typename... TArgs>
			static TDerived Read(const TArgs... Args) {
				static_assert(RawAccessible<TDerived, TValue, TArgs...>, "Register must define static ReadRaw/WriteRaw");
				TDerived reg{};
				reg.All = TDerived::ReadRaw(Args...);
				return reg;
			}

			template <typename... TArgs>
			void Write(const TArgs... Args) {
				static_assert(RawAccessible<TDerived, TValue, TArgs...>, "Register must define static ReadRaw/WriteRaw");
				TDerived::WriteRaw(All, Args...);
			}

			friend TDerived;
			
			template <typename TTDerived, typename TTBits, MsrAddress TAddress>
			friend struct Msr;	
			
			template <typename TTDerived, typename TTBits, u32 TIndex>
			friend struct Dr;
			
			template <typename TTDerived, typename TTBits, MsrAddress TFirst, u32 TStride>
			friend struct IndexedMsr;
		};

		enum class MsrAddress : u32 {
			ApicBase = 0x1b,
			Ia32FeatureControl = 0x3a,
			Ia32MtrrCapabilities = 0xfe,
			
			Ia32MtrrDefType = 0x2ff,

			Ia32VmxBasic = 0x480,
			Ia32VmxPinBasedControls = 0x481,
			Ia32VmxProcBasedControls = 0x482,
			Ia32VmxExitControls = 0x483,
			Ia32VmxEntryControls = 0x484,
			Ia32VmxMisc = 0x485,
			Ia32VmxCr0Fixed0 = 0x486,
			Ia32VmxCr0Fixed1 = 0x487,
			Ia32VmxCr4Fixed0 = 0x488,
			Ia32VmxCr4Fixed1 = 0x489,
			Ia32VmxVmcsEnum = 0x48a,
			Ia32VmxProcBasedControls2 = 0x48b,
			Ia32VmxEptVpidCap = 0x48c,
			Ia32VmxTruePinBasedControls = 0x48d,
			Ia32VmxTrueProcBasedControls = 0x48e,
			Ia32VmxTrueExitControls = 0x48f,
			Ia32VmxTrueEntryControls = 0x490,
			Ia32VmxVmFunc = 0x491,

			Ia32SysenterCs = 0x174,
			Ia32SysenterEsp = 0x175,
			Ia32SysenterEip = 0x176,

			Ia32Debugctl = 0x1d9,
			
			Ia32MtrrPhysBase0 = 0x200,
			Ia32MtrrPhysMask0 = 0x201,
			Ia32MtrrPhysBase1 = 0x202,
			Ia32MtrrPhysMask1 = 0x203,
			Ia32MtrrPhysBase2 = 0x204,
			Ia32MtrrPhysMask2 = 0x205,
			Ia32MtrrPhysBase3 = 0x206,
			Ia32MtrrPhysMask3 = 0x207,
			Ia32MtrrPhysBase4 = 0x208,
			Ia32MtrrPhysMask4 = 0x209,
			Ia32MtrrPhysBase5 = 0x20a,
			Ia32MtrrPhysMask5 = 0x20b,
			Ia32MtrrPhysBase6 = 0x20c,
			Ia32MtrrPhysMask6 = 0x20d,
			Ia32MtrrPhysBase7 = 0x20e,
			Ia32MtrrPhysMask7 = 0x20f,
			Ia32MtrrPhysBase8 = 0x210,
			Ia32MtrrPhysMask8 = 0x211,
			Ia32MtrrPhysBase9 = 0x212,
			Ia32MtrrPhysMask9 = 0x213,

			FsBase = 0xc0000100,
			GsBase = 0xc0000101,
			ShadowGsBase = 0xc0000102,
		};
		
		union MsrHalves {
			u64 All;
			struct {
				u32 low;
				u32 high;
			} Parts;
		};

		template <typename TDerived, typename TBits, MsrAddress TAddress>
		struct Msr : RegisterBase<TDerived, TBits, u64> {
		private:
			Msr() = default;

		public:
			static u64 ReadRaw() {
				return __readmsr(static_cast<u32>(TAddress));
			}

			static void WriteRaw(const u64 Value) {
				__writemsr(static_cast<u32>(TAddress), Value);
			}

			friend TDerived;
		};
		
		template <typename TDerived, typename TBits, MsrAddress TFirst, u32 TStride>
		struct IndexedMsr : RegisterBase<TDerived, TBits, u64> {
		private:
			IndexedMsr() = default;
			
			static constexpr u32 Address(const u32 Index) {
				return static_cast<u32>(TFirst) + Index * TStride;
			}
			
		public:
			static u64 ReadRaw(const u32 Index) {
				return __readmsr(Address(Index));
			}
			
			static void WriteRaw(const u64 Value, const u32 Index) {
				__writemsr(Address(Index), Value);
			}
			
			friend TDerived;
		};

		struct Cr0 : RegisterBase<Cr0, u64, u64> {
			static u64 ReadRaw() {
				return __readcr0();
			}

			static void WriteRaw(const u64 Value) {
				__writecr0(Value);
			}
		};

		struct Cr3 : RegisterBase<Cr3, u64, u64> {
			static constexpr u64 g_pcidNoInvalidate = 1ULL << 63;
			static u64 ReadRaw() {
				return __readcr3();
			}

			static void WriteRaw(const u64 Value) {
				__writecr3(Value);
			}
		};

		struct Cr4Bits {
			u64 vme : 1;
			u64 pvi : 1;
			u64 tsd : 1;
			u64 de : 1;
			u64 pse : 1;
			u64 pae : 1;
			u64 mce : 1;
			u64 pge : 1;
			u64 pce : 1;
			u64 osfxsr : 1;
			u64 osxmmexcpt : 1;
			u64 umip : 1;
			u64 la57 : 1;
			u64 vmxe : 1;
			u64 smxe : 1;
			u64 reserved0 : 1;
			u64 fsgsbase : 1;
			u64 pcide : 1;
			u64 osxsave : 1;
			u64 kl : 1;
			u64 smep : 1;
			u64 smap : 1;
			u64 pke : 1;
			u64 cet : 1;
			u64 pks : 1;
			u64 uintr : 1;
			u64 reserved1 : 1;
			u64 lass : 1;
			u64 lamSup : 1;
			u64 reserved2 : 35;
		};

		struct Cr4 : RegisterBase<Cr4, Cr4Bits, u64> {
			static u64 ReadRaw() {
				return __readcr4();
			}

			static void WriteRaw(const u64 Value) {
				__writecr4(Value);
			}
		};

		struct FeatureControlBits {
			u64 lock : 1;
			u64 vmxInsideSmx : 1;
			u64 vmxOutsideSmx : 1;
			u64 reserved0 : 5;
			u64 senterLocalFunctions : 7;
			u64 senterGlobalEnable : 1;
			u64 reserved1 : 1;
			u64 sgxLaunchControl : 1;
			u64 sgxGlobalEnable : 1;
			u64 reserved2 : 1;
			u64 lmceOn : 1;
			u64 reserved3 : 43;
		};

		struct Ia32FeatureControl : Msr<Ia32FeatureControl, FeatureControlBits, MsrAddress::Ia32FeatureControl> {};

		struct VmxBasicBits {
			u64 revision : 31;
			u64 reserved0 : 1;
			u64 regionSize : 13;
			u64 reserved1 : 3;
			u64 physAddrWidth : 1;
			u64 dualMode : 1;
			u64 memoryType : 4;
			u64 insOutsReports : 1;
			u64 trueControls : 1;
			u64 hwExcDelivery : 1;
			u64 reserved2 : 1;
			u64 nestedExcSupport : 1;
			u64 reserved3 : 5;
		};

		struct Ia32VmxBasic : Msr<Ia32VmxBasic, VmxBasicBits, MsrAddress::Ia32VmxBasic> {};

		struct Ia32Debugctl : Msr<Ia32Debugctl, u64, MsrAddress::Ia32Debugctl> {};

		struct FsBase : Msr<FsBase, u64, MsrAddress::FsBase> {};
		struct GsBase : Msr<GsBase, u64, MsrAddress::GsBase> {};

		struct Ia32SysenterCs : Msr<Ia32SysenterCs, u64, MsrAddress::Ia32SysenterCs> {};
		struct Ia32SysenterEip : Msr<Ia32SysenterEip, u64, MsrAddress::Ia32SysenterEip> {};
		struct Ia32SysenterEsp : Msr<Ia32SysenterEsp, u64, MsrAddress::Ia32SysenterEsp> {};
		
		struct Ia32MtrrDefTypeBits {
			u64 defaultMemoryType : 3;
			u64 reserved0 : 7;
			u64 fixedRangeMtrrEnable : 1;
			u64 mtrrEnable : 1;
			u64 reserved1 : 52;
		};
		struct Ia32MtrrDefType : Msr<Ia32MtrrDefType, Ia32MtrrDefTypeBits, MsrAddress::Ia32MtrrDefType> {};
		
		struct Ia32MtrrCapabilitiesBits {
			u64 variableRangeCount : 8;
			u64 fixedRangeSupported : 1;
			u64 reserved0 : 1;
			u64 wcSupported : 1;
			u64 smrrSupported : 1;
			u64 reserved1 : 52;
		};
		struct Ia32MtrrCapabilities : Msr<Ia32MtrrCapabilities, Ia32MtrrCapabilitiesBits, MsrAddress::Ia32MtrrCapabilities> {};
		
		struct Ia32MtrrPhysBaseBits {
			u64 type : 8;
			u64 reserved0 : 4;
			u64 pageFrameNumber : 36;
			u64 reserved1 : 16;
		};
		struct Ia32MtrrPhysBase : IndexedMsr<Ia32MtrrPhysBase, Ia32MtrrPhysBaseBits, MsrAddress::Ia32MtrrPhysBase0, 2> {};
		
		struct Ia32MtrrPhysMaskBits {
			u64 type : 8;
			u64 reserved0 : 3;
			u64 valid : 1;
			u64 pageFrameNumber : 36;
			u64 reserved1 : 16;
		};
		struct Ia32MtrrPhysMask : IndexedMsr<Ia32MtrrPhysMask, Ia32MtrrPhysMaskBits, MsrAddress::Ia32MtrrPhysMask0, 2> {};
		
		template <typename TDerived, typename TBits, u32 TIndex>
		struct Dr : RegisterBase<TDerived, TBits, u64> {
		private:
			Dr() = default;

		public:
			static u64 ReadRaw() {
				return __readdr(TIndex);
			}

			static void WriteRaw(const u64 Value) {
				__writedr(TIndex, Value);
			}

			friend TDerived;
		};
		
		struct Dr7 : Dr<Dr7, u64, 7> {};
		
		struct RflagsBits {
			u64 carry : 1;
			u64 reserved0 : 1;
			u64 parity : 1;
			u64 reserved1 : 1;
			u64 auxiliaryCarry : 1;
			u64 reserved2 : 1;
			u64 zero : 1;
			u64 sign : 1;
			u64 trap : 1;
			u64 interrupt : 1;
			u64 direction : 1;
			u64 overflow : 1;
			u64 ioPrivilegeLevel : 2;
			u64 nestedTask : 1;
			u64 reserved3 : 1;
			u64 resume : 1;
			u64 virtual8086 : 1;
			u64 alignmentCheck : 1;
			u64 virtualInterrupt : 1;
			u64 virtualInterruptPending : 1;
			u64 cpuid : 1;
			u64 reserved4 : 42;
		};
		
		struct Rflags : RegisterBase<Rflags, RflagsBits, u64> {
			static u64 ReadRaw() {
				return __readeflags();
			}
			
			static void WriteRaw(const u64 Value) {
				__writeeflags(Value);
			}
		};
	}

	namespace Leaves {
		template <typename TBits>
		concept CpuidRegisterBits = sizeof(TBits) == sizeof(u32);

		template <typename TDerived, i32 TLeaf, i32 TSubleaf = 0>
		struct CpuidLeafBase {
		private:
			CpuidLeafBase() = default;

		public:
			static Cpuid::Data Query() {
				Cpuid::Data result{};
				__cpuidex(bit_cast<i32*>(&result), TLeaf, TSubleaf);
				return result;
			}

			template <CpuidRegisterBits TBits>
			static TBits As(u32 Cpuid::Data::* Reg) {
				return bit_cast<TBits>(Query().*Reg);
			}

			friend TDerived;
		};

		struct Leaf1EcxBits {
			u32 unused0 : 5;
			u32 vmx : 1;
			u32 unused1 : 26;
		};

		struct Leaf1 : CpuidLeafBase<Leaf1, 1> {
			static Leaf1EcxBits Ecx() {
				return As<Leaf1EcxBits>(&Cpuid::Data::ecx);
			}
		};
	}

	namespace Vmx {
		using VmxRegion = Memory::ContiguousAllocation<PAGE_SIZE>;
		
		bool CheckVmx();

		void EnableVmxInFeatureControl();
		
		[[nodiscard]]
		Registers::Cr0 WithFixedBits(_In_ Registers::Cr0 Cr0);
		
		[[nodiscard]]
		Registers::Cr4 WithFixedBits(_In_ Registers::Cr4 Cr4);

		Expected<void> VmxOn(
			_In_ const VmxRegion& Region
		);

		Expected<void> VmPtrLd(
			_In_ const VmxRegion& Region
		);

		void VmxOff();

		void* VmPtrSt();

		[[nodiscard]]
		Expected<void> VmClear(
			_In_ const VmxRegion& Region
		);

		[[nodiscard]]
		Err VmLaunch();

		u64 VmLastError();

		Expected<u64> VmRead(
			_In_ VMCS::Field Field
		);
		
		template <typename T>
		[[nodiscard]]
		Expected<T> VmReadAs(
			_In_ const VMCS::Field Field
		) {
			TRY_ASSIGN(const u64 raw, VmRead(Field));
			
			if constexpr (sizeof(T) == sizeof(u64)) {
				return Ok(bit_cast<T>(raw));
			} else {
				return Ok(bit_cast<T>(static_cast<u32>(raw)));
			}
		}

		Expected<void> VmWrite(
			_In_ VMCS::Field Field,
			_In_ u64 Value
		);
	}
	
	namespace Msr {
		u64 Read(
			_In_ u32 Register
		);
		
		void Write(
			_In_ u32 Register,
			_In_ u64 Value
		);
	}
}