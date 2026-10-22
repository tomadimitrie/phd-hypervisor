#include "VMCS.hpp"
#include "Hypervisor.hpp"

#include "Common/Utils.hpp"
#include "Logging.hpp"
#include "CPU.hpp"
#include "AsmHelpersC.hpp" 
#include "Vcpu.hpp"


namespace VMCS {
	Expected<PrivilegeLevel> GetGuestCpl() {
		TRY_ASSIGN(const auto ssAr, Cpu::Vmx::VmReadAs<SegmentAccessRights>(Field::GuestSsArBytes));
		return Ok(ssAr.Fields.dpl);
	}
}

namespace {
	Expected<VMCS::SegmentInfo> GetSegmentDescriptor(
		_In_ const u64 Selector,
		_In_ void* GdtBase
	) {
		const VMCS::SegmentSelector segmentSelector{
			.All = static_cast<u16>(Selector),
		};

		if (Selector == 0) {
			return Err(STATUS_INVALID_PARAMETER);
		}

		if (segmentSelector.Fields.tableIndicator) {
			return Err(STATUS_INVALID_PARAMETER);
		}

		const auto descriptor = bit_cast<VMCS::SegmentDescriptor*>(GdtBase) + segmentSelector.Fields.index;
		const auto& fields = descriptor->Fields;
		VMCS::SegmentInfo info{};
		info.selector = static_cast<u16>(Selector);
		info.base = static_cast<u64>(fields.baseLow) | static_cast<u64>(fields.baseHigh) << 24;
		info.limit = static_cast<u32>(fields.limitLow | fields.limitHigh << 16);
		
		if (!fields.descriptorType) {
			const auto baseUpper = *bit_cast<const u32*>(descriptor + 1);
			info.base |= static_cast<u64>(baseUpper) << 32;
		}
		
		if (fields.granularity) {
			info.limit = (info.limit << PAGE_SHIFT) | (PAGE_SIZE - 1);
		}
		
		info.accessRights.Fields = {
			.type = static_cast<u32>(fields.type),
			.descriptorType = static_cast<u32>(fields.descriptorType),
			.dpl = static_cast<VMCS::PrivilegeLevel>(fields.dpl),
			.present = static_cast<u32>(fields.present),
			.available = static_cast<u32>(fields.available),
			.longMode = static_cast<u32>(fields.longMode),
			.defaultBig = static_cast<u32>(fields.defaultBig),
			.granularity = static_cast<u32>(fields.granularity),
		};
		
		return Ok(info);
	}

	[[nodiscard]]
	Expected<void> FillGuestSelectorData(
		_In_ void* GdtBase,
		_In_ VMCS::Segment Segment,
		_In_ const u64 Selector
	) {
		VMCS::SegmentInfo segment{}; 
		
		if (Selector == 0) {
			segment.accessRights.Fields.unusable = true;
		} else {
			TRY_ASSIGN(segment, GetSegmentDescriptor(Selector, GdtBase));
		}
		
		const auto fieldOffset = static_cast<u32>(Segment) * 2;
		const auto fieldFor = [fieldOffset](const VMCS::Field EsField) {
			return static_cast<VMCS::Field>(static_cast<u32>(EsField) + fieldOffset);
		};

		using enum VMCS::Field;
		
		TRY(Cpu::Vmx::VmWrite(fieldFor(GuestEsSelector), Selector));
		TRY(Cpu::Vmx::VmWrite(fieldFor(GuestEsLimit), segment.limit));
		TRY(Cpu::Vmx::VmWrite(fieldFor(GuestEsArBytes), segment.accessRights.All));
		TRY(Cpu::Vmx::VmWrite(fieldFor(GuestEsBase), segment.base));
		
		return Ok();
	}

	u32 AdjustControls(
		_In_ u32 Control,
		_In_ Cpu::Registers::MsrAddress Msr
	) {
		const Cpu::Registers::MsrHalves msr = {
			.All = __readmsr(static_cast<u32>(Msr)),
		};
		Control &= msr.Parts.high;
		Control |= msr.Parts.low;
		return Control;
	}
	
	[[nodiscard]]
	u16 ToHostSelector(
		_In_ const u64 Selector
	) {
		VMCS::SegmentSelector selector{
			.All = static_cast<u16>(Selector)
		};
		selector.Fields.rpl = 0;
		selector.Fields.tableIndicator = 0;
		return selector.All;
	}
}

_Use_decl_annotations_
Expected<void> Vcpu::SetupVmcs(
	void* GuestStack
) {
	using namespace VMCS;
	using enum Field;
	using MsrAddress = Cpu::Registers::MsrAddress;
	
	const auto pinBasedMsr = MsrAddress::Ia32VmxPinBasedControls;
	const auto procBasedMsr = MsrAddress::Ia32VmxProcBasedControls;
	const auto exitMsr = MsrAddress::Ia32VmxExitControls;
	const auto entryMsr = MsrAddress::Ia32VmxEntryControls;
	
	const auto gdtBase = bit_cast<void*>(GetGdtBase());
	const auto debugctl = Cpu::Registers::Ia32Debugctl::Read();
	
	TRY_ASSIGN(const auto tr, GetSegmentDescriptor(GetTr(), gdtBase));

	TRY(Cpu::Vmx::VmWrite(HostEsSelector, ToHostSelector(GetESValue())));
	TRY(Cpu::Vmx::VmWrite(HostCsSelector, ToHostSelector(GetCSValue())));
	TRY(Cpu::Vmx::VmWrite(HostSsSelector, ToHostSelector(GetSSValue())));
	TRY(Cpu::Vmx::VmWrite(HostDsSelector, ToHostSelector(GetDSValue())));
	TRY(Cpu::Vmx::VmWrite(HostFsSelector, ToHostSelector(GetFSValue())));
	TRY(Cpu::Vmx::VmWrite(HostGsSelector, ToHostSelector(GetGSValue())));
	TRY(Cpu::Vmx::VmWrite(HostTrSelector, ToHostSelector(GetTr())));
	
	TRY(Cpu::Vmx::VmWrite(VmcsLinkPointer, ~0ULL));

	TRY(Cpu::Vmx::VmWrite(GuestIa32Debugctl, debugctl.All));
	
	TRY(Cpu::Vmx::VmWrite(TscOffset, 0));

	TRY(Cpu::Vmx::VmWrite(PageFaultErrorCodeMask, 0));
	TRY(Cpu::Vmx::VmWrite(PageFaultErrorCodeMatch, 0));

	TRY(Cpu::Vmx::VmWrite(VmExitMsrStoreCount, 0));
	TRY(Cpu::Vmx::VmWrite(VmExitMsrLoadCount, 0));

	TRY(Cpu::Vmx::VmWrite(VmEntryMsrLoadCount, 0));
	TRY(Cpu::Vmx::VmWrite(VmEntryIntrInfoField, 0));

	TRY(FillGuestSelectorData(gdtBase, Segment::Es, GetESValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Cs, GetCSValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Ss, GetSSValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Ds, GetDSValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Fs, GetFSValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Gs, GetGSValue()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Ldtr, GetLdtr()));
	TRY(FillGuestSelectorData(gdtBase, Segment::Tr, GetTr()));

	TRY(Cpu::Vmx::VmWrite(GuestFsBase, Cpu::Registers::FsBase::Read().All));
	TRY(Cpu::Vmx::VmWrite(GuestGsBase, Cpu::Registers::GsBase::Read().All));

	TRY(Cpu::Vmx::VmWrite(GuestInterruptibilityInfo, 0));
	TRY(Cpu::Vmx::VmWrite(GuestActivityState, 0));

	TRY(Cpu::Vmx::VmWrite(
		CpuBasedVmExecControl, 
		AdjustControls(
			static_cast<u32>(
				PrimaryControls::ActivateSecondaryControls | 
				PrimaryControls::ActivateMsrBitmaps
			),
			procBasedMsr
		)
	));
	TRY(Cpu::Vmx::VmWrite(
		SecondaryVmExecControl, 
		AdjustControls(
			static_cast<u32>(
				SecondaryControls::EnableRdtscp |
				SecondaryControls::EnableInvpcid |
				SecondaryControls::EnableXsaves | 
				SecondaryControls::EnableUserWaitPause
			),
			Cpu::Registers::MsrAddress::Ia32VmxProcBasedControls2
		)
	));

	TRY(Cpu::Vmx::VmWrite(
		PinBasedVmExecControl, 
		AdjustControls(
			0,
			pinBasedMsr
		)
	));

	TRY(Cpu::Vmx::VmWrite(
		VmExitControls, 
		AdjustControls(
			static_cast<u32>(
				VmExitControls::SaveDebugControls |
				VmExitControls::Ia32EMode | 
				VmExitControls::AckIntrOnExit
			),
			exitMsr
		)
	));
	TRY(Cpu::Vmx::VmWrite(
		VmEntryControls, 
		AdjustControls(
			static_cast<u32>(
				VmEntryControls::LoadDebugControls |
				VmEntryControls::Ia32AMode
			),
			entryMsr
		)
	));
	
	
	TRY(Cpu::Vmx::VmWrite(Cr3TargetCount, 0));
	// TRY(Cpu::Vmx::VmWrite(Cr3TargetValue0, 0));
	// TRY(Cpu::Vmx::VmWrite(Cr3TargetValue1, 0));
	// TRY(Cpu::Vmx::VmWrite(Cr3TargetValue2, 0));
	// TRY(Cpu::Vmx::VmWrite(Cr3TargetValue3, 0));
	
	TRY(Cpu::Vmx::VmWrite(Cr0GuestHostMask, 0));
	TRY(Cpu::Vmx::VmWrite(Cr4GuestHostMask, 0));
	TRY(Cpu::Vmx::VmWrite(Cr0ReadShadow, 0));
	TRY(Cpu::Vmx::VmWrite(Cr4ReadShadow, 0));

	TRY(Cpu::Vmx::VmWrite(GuestCr0, Cpu::Registers::Cr0::Read().All));
	TRY(Cpu::Vmx::VmWrite(GuestCr3, Cpu::Registers::Cr3::Read().All));
	TRY(Cpu::Vmx::VmWrite(GuestCr4, Cpu::Registers::Cr4::Read().All));
	
	TRY(Cpu::Vmx::VmWrite(GuestDr7, Cpu::Registers::Dr7::Read().All));

	TRY(Cpu::Vmx::VmWrite(HostCr0, Cpu::Registers::Cr0::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostCr3, Cpu::Registers::Cr3::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostCr4, Cpu::Registers::Cr4::Read().All));

	TRY(Cpu::Vmx::VmWrite(GuestGdtrBase, GetGdtBase()));
	TRY(Cpu::Vmx::VmWrite(GuestGdtrLimit, GetGdtLimit()));
	TRY(Cpu::Vmx::VmWrite(GuestIdtrBase, GetIdtBase()));
	TRY(Cpu::Vmx::VmWrite(GuestIdtrLimit, GetIdtLimit()));
	
	TRY(Cpu::Vmx::VmWrite(GuestRflags, Cpu::Registers::Rflags::Read().All));

	TRY(Cpu::Vmx::VmWrite(GuestSysenterCs, Cpu::Registers::Ia32SysenterCs::Read().All));
	TRY(Cpu::Vmx::VmWrite(GuestSysenterEip, Cpu::Registers::Ia32SysenterEip::Read().All));
	TRY(Cpu::Vmx::VmWrite(GuestSysenterEsp, Cpu::Registers::Ia32SysenterEsp::Read().All));
	
	TRY(Cpu::Vmx::VmWrite(HostTrBase, tr.base));
	TRY(Cpu::Vmx::VmWrite(HostFsBase, Cpu::Registers::FsBase::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostGsBase, Cpu::Registers::GsBase::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostGdtrBase, GetGdtBase()));
	TRY(Cpu::Vmx::VmWrite(HostIdtrBase, GetIdtBase()));
	
	TRY(Cpu::Vmx::VmWrite(HostIa32SysenterCs, Cpu::Registers::Ia32SysenterCs::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostIa32SysenterEip, Cpu::Registers::Ia32SysenterEip::Read().All));
	TRY(Cpu::Vmx::VmWrite(HostIa32SysenterEsp, Cpu::Registers::Ia32SysenterEsp::Read().All));
	
	TRY(Cpu::Vmx::VmWrite(MsrBitmap, msrBitmap.Physical()));

	TRY(Cpu::Vmx::VmWrite(GuestRsp, bit_cast<u64>(GuestStack)));
	TRY(Cpu::Vmx::VmWrite(GuestRip, bit_cast<u64>(&AsmPopState)));

	TRY(Cpu::Vmx::VmWrite(HostRsp, bit_cast<u64>(stack.Virtual()) + stack.Size()));
	TRY(Cpu::Vmx::VmWrite(HostRip, bit_cast<u64>(&AsmVmExitHandler)));
	
	return Ok();
}
