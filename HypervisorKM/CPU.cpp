#include "CPU.hpp"

#include <intrin.h>

#include "Common/Utils.hpp"
#include "Logging.hpp"
#include "VMCS.hpp"

namespace Cpu {
	namespace Cpuid {
		_Use_decl_annotations_
		Data Query(
			const int Leaf
		) {
			Data data{};
			__cpuid(bit_cast<int*>(&data), Leaf);
			return data;
		}
		
		_Use_decl_annotations_
		Data QueryEx(
			const int Leaf,
			const int Subleaf
		) {
			Data data{};
			__cpuidex(bit_cast<int*>(&data), Leaf, Subleaf);
			return data;
		}

		_Use_decl_annotations_
		void GetCpuVendor(
			unsigned char Vendor[]
		) {
			const auto info = Query(0);

			CopyPackedMemory(
				Vendor,
				info.ebx,
				info.edx,
				info.ecx
			);
		}
	}

	namespace Vmx {
		namespace {
			u64 ApplyVmxFixedBits(
				_In_ const u64 Value,
				_In_ const Registers::MsrAddress Fixed0,
				_In_ const Registers::MsrAddress Fixed1
			) {
				const u64 mustBeOne = Cpu::Msr::Read(static_cast<u32>(Fixed0));
				const u64 mayBeOne = Cpu::Msr::Read(static_cast<u32>(Fixed1));
				return (Value | mustBeOne) & mayBeOne;
			}
		}
		
		_Use_decl_annotations_
		Registers::Cr0
		WithFixedBits(Registers::Cr0 Cr0) {
			using enum Registers::MsrAddress;
			
			Cr0.All = ApplyVmxFixedBits(Cr0.All, Ia32VmxCr0Fixed0, Ia32VmxCr0Fixed1);
			return Cr0;
		}
		
		_Use_decl_annotations_
		Registers::Cr4
		WithFixedBits(Registers::Cr4 Cr4) {
			using enum Registers::MsrAddress;
			
			Cr4.All = ApplyVmxFixedBits(Cr4.All, Ia32VmxCr4Fixed0, Ia32VmxCr4Fixed1);
			return Cr4;
		}

		bool CheckVmx() {
			if (!Leaves::Leaf1::Ecx().vmx) {
				HvFail("VMX is disabled");
				return false;
			}

			if (
				const auto control = Registers::Ia32FeatureControl::Read(); 
				control.Fields.lock && !control.Fields.vmxOutsideSmx
			) {
				HvFail("VMX is disabled and locked");
				return false;
			}

			return true;
		}

		void EnableVmxInFeatureControl() {
			auto control = Registers::Ia32FeatureControl::Read(); 
			if (control.Fields.lock) {
				return;
			}
			control.Fields.lock = true;
			control.Fields.vmxOutsideSmx = true;
			control.Write();
		}

		_Use_decl_annotations_
		Expected<void> VmxOn(
			const VmxRegion& Region
		) {
			u64 physical = Region.Physical();
			if (__vmx_on(&physical) != 0) {
				return Err(STATUS_UNSUCCESSFUL);
			}
			return Ok();
		}

		_Use_decl_annotations_
		Expected<void> VmPtrLd(
			const VmxRegion& Region
		) {
			u64 physical = Region.Physical();
			if (__vmx_vmptrld(&physical) != 0) {
				return Err(STATUS_UNSUCCESSFUL);
			}
			return Ok();
		}

		void VmxOff() {
			__vmx_off();
		}

		void* VmPtrSt() {
			u64 addr = 0;
			__vmx_vmptrst(&addr);
			return bit_cast<void*>(addr);
		}

		_Use_decl_annotations_
		Expected<void> VmClear(
			const VmxRegion& Region
		) {
			u64 physical = Region.Physical();
			if (__vmx_vmclear(&physical) != 0) {
				return Err(STATUS_UNSUCCESSFUL);
			}
			return Ok();
		}

		Err VmLaunch() {
			(void)__vmx_vmlaunch();
			return Err(STATUS_UNSUCCESSFUL);
		}

		u64 VmLastError() {
			return VmRead(VMCS::Field::VmInstructionError).ValueOr(0);
		}

		_Use_decl_annotations_
		Expected<u64> VmRead(
			const VMCS::Field Field
		) {
			u64 value = 0;
			if (__vmx_vmread(static_cast<u64>(Field), &value) != 0) {
				return Err(STATUS_UNSUCCESSFUL);
			}
			return Ok(value);
		}

		_Use_decl_annotations_
		Expected<void> VmWrite(
			const VMCS::Field Field,
			const u64 Value
		) {
			if (__vmx_vmwrite(static_cast<u64>(Field), Value) != 0) {
				HvFail("VMWrite field 0x%x failed with error 0x%llx", static_cast<u32>(Field), VmLastError());
				return Err(STATUS_UNSUCCESSFUL);	
			}
			return Ok();
		}
	}

	namespace Msr {
		_Use_decl_annotations_
		u64 Read(
			const u32 Register
		) {
			return __readmsr(Register);	
		}
		
		_Use_decl_annotations_
		void Write(
			const u32 Register,
			const u64 Value
		) {
			__writemsr(Register, Value);
		}
	}
}