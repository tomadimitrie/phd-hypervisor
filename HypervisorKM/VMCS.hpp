#pragma once

#include "Memory.hpp"

namespace VMCS {
    enum class Field : u32 {
        GuestEsSelector = 0x00000800,
        GuestCsSelector = 0x00000802,
        GuestSsSelector = 0x00000804,
        GuestDsSelector = 0x00000806,
        GuestFsSelector = 0x00000808,
        GuestGsSelector = 0x0000080a,
        GuestLdtrSelector = 0x0000080c,
        GuestTrSelector = 0x0000080e,
        HostEsSelector = 0x00000c00,
        HostCsSelector = 0x00000c02,
        HostSsSelector = 0x00000c04,
        HostDsSelector = 0x00000c06,
        HostFsSelector = 0x00000c08,
        HostGsSelector = 0x00000c0a,
        HostTrSelector = 0x00000c0c,
        IoBitmapA = 0x00002000,
        IoBitmapAHigh = 0x00002001,
        IoBitmapB = 0x00002002,
        IoBitmapBHigh = 0x00002003,
        MsrBitmap = 0x00002004,
        MsrBitmapHigh = 0x00002005,
        VmExitMsrStoreAddr = 0x00002006,
        VmExitMsrStoreAddrHigh = 0x00002007,
        VmExitMsrLoadAddr = 0x00002008,
        VmExitMsrLoadAddrHigh = 0x00002009,
        VmEntryMsrLoadAddr = 0x0000200a,
        VmEntryMsrLoadAddrHigh = 0x0000200b,
        TscOffset = 0x00002010,
        TscOffsetHigh = 0x00002011,
        VirtualApicPageAddr = 0x00002012,
        VirtualApicPageAddrHigh = 0x00002013,
        VmfuncControls = 0x00002018,
        VmfuncControlsHigh = 0x00002019,
        EptPointer = 0x0000201A,
        EptPointerHigh = 0x0000201B,
        EptpList = 0x00002024,
        EptpListHigh = 0x00002025,
        GuestPhysicalAddress = 0x2400,
        GuestPhysicalAddressHigh = 0x2401,
        VmcsLinkPointer = 0x00002800,
        VmcsLinkPointerHigh = 0x00002801,
        GuestIa32Debugctl = 0x00002802,
        GuestIa32DebugctlHigh = 0x00002803,
        PinBasedVmExecControl = 0x00004000,
        CpuBasedVmExecControl = 0x00004002,
        ExceptionBitmap = 0x00004004,
        PageFaultErrorCodeMask = 0x00004006,
        PageFaultErrorCodeMatch = 0x00004008,
        Cr3TargetCount = 0x0000400a,
        VmExitControls = 0x0000400c,
        VmExitMsrStoreCount = 0x0000400e,
        VmExitMsrLoadCount = 0x00004010,
        VmEntryControls = 0x00004012,
        VmEntryMsrLoadCount = 0x00004014,
        VmEntryIntrInfoField = 0x00004016,
        VmEntryExceptionErrorCode = 0x00004018,
        VmEntryInstructionLen = 0x0000401a,
        TprThreshold = 0x0000401c,
        SecondaryVmExecControl = 0x0000401e,
        VmInstructionError = 0x00004400,
        VmExitReason = 0x00004402,
        VmExitIntrInfo = 0x00004404,
        VmExitIntrErrorCode = 0x00004406,
        IdtVectoringInfoField = 0x00004408,
        IdtVectoringErrorCode = 0x0000440a,
        VmExitInstructionLen = 0x0000440c,
        VmxInstructionInfo = 0x0000440e,
        GuestEsLimit = 0x00004800,
        GuestCsLimit = 0x00004802,
        GuestSsLimit = 0x00004804,
        GuestDsLimit = 0x00004806,
        GuestFsLimit = 0x00004808,
        GuestGsLimit = 0x0000480a,
        GuestLdtrLimit = 0x0000480c,
        GuestTrLimit = 0x0000480e,
        GuestGdtrLimit = 0x00004810,
        GuestIdtrLimit = 0x00004812,
        GuestEsArBytes = 0x00004814,
        GuestCsArBytes = 0x00004816,
        GuestSsArBytes = 0x00004818,
        GuestDsArBytes = 0x0000481a,
        GuestFsArBytes = 0x0000481c,
        GuestGsArBytes = 0x0000481e,
        GuestLdtrArBytes = 0x00004820,
        GuestTrArBytes = 0x00004822,
        GuestInterruptibilityInfo = 0x00004824,
        GuestActivityState = 0x00004826,
        GuestSmBase = 0x00004828,
        GuestSysenterCs = 0x0000482A,
        HostIa32SysenterCs = 0x00004c00,
        Cr0GuestHostMask = 0x00006000,
        Cr4GuestHostMask = 0x00006002,
        Cr0ReadShadow = 0x00006004,
        Cr4ReadShadow = 0x00006006,
        Cr3TargetValue0 = 0x00006008,
        Cr3TargetValue1 = 0x0000600a,
        Cr3TargetValue2 = 0x0000600c,
        Cr3TargetValue3 = 0x0000600e,
        ExitQualification = 0x00006400,
        GuestLinearAddress = 0x0000640a,
        GuestCr0 = 0x00006800,
        GuestCr3 = 0x00006802,
        GuestCr4 = 0x00006804,
        GuestEsBase = 0x00006806,
        GuestCsBase = 0x00006808,
        GuestSsBase = 0x0000680a,
        GuestDsBase = 0x0000680c,
        GuestFsBase = 0x0000680e,
        GuestGsBase = 0x00006810,
        GuestLdtrBase = 0x00006812,
        GuestTrBase = 0x00006814,
        GuestGdtrBase = 0x00006816,
        GuestIdtrBase = 0x00006818,
        GuestDr7 = 0x0000681a,
        GuestRsp = 0x0000681c,
        GuestRip = 0x0000681e,
        GuestRflags = 0x00006820,
        GuestPendingDbgExceptions = 0x00006822,
        GuestSysenterEsp = 0x00006824,
        GuestSysenterEip = 0x00006826,
        HostCr0 = 0x00006c00,
        HostCr3 = 0x00006c02,
        HostCr4 = 0x00006c04,
        HostFsBase = 0x00006c06,
        HostGsBase = 0x00006c08,
        HostTrBase = 0x00006c0a,
        HostGdtrBase = 0x00006c0c,
        HostIdtrBase = 0x00006c0e,
        HostIa32SysenterEsp = 0x00006c10,
        HostIa32SysenterEip = 0x00006c12,
        HostRsp = 0x00006c14,
        HostRip = 0x00006c16,
    };
	
	union SegmentSelector {
		u16 All;
		struct {
			u16 rpl : 2;
			u16 tableIndicator : 1;
			u16 index : 13;
		} Fields;
		static_assert(sizeof(Fields) == sizeof(All));
	};
	
    union SegmentDescriptor {
    	u64 All;
    	struct {
    		u64 limitLow : 16;
    		u64 baseLow : 24;
    		u64 type : 4;
    		u64 descriptorType : 1;
    		u64 dpl : 2;
    		u64 present : 1;
    		u64 limitHigh : 4;
    		u64 available : 1;
    		u64 longMode : 1;
    		u64 defaultBig : 1;
    		u64 granularity : 1;
    		u64 baseHigh : 8;
    	} Fields;
		static_assert(sizeof(Fields) == sizeof(All));
    };
	static_assert(sizeof(SegmentDescriptor) == sizeof(u64));
	
    enum class Segment : u8 {
        Es = 0,
        Cs = 1,
        Ss = 2,
        Ds = 3,
        Fs = 4,
        Gs = 5,
        Ldtr = 6,
        Tr = 7,
    };

    enum class PrimaryControls : u32 {
        VirtualInterruptPending = 1UL << 2,
        UseTscOffsetting = 1UL << 3,
        HltExiting = 1UL << 7,
        InvlpgExiting = 1UL << 9,
        MwaitExiting = 1UL << 10,
        RdpmcExiting = 1UL << 11,
        RdtscExiting = 1UL << 12,
        Cr3LoadExiting = 1UL << 15,
        Cr3StoreExiting = 1UL << 16,
        Cr8LoadExiting = 1UL << 19,
        Cr8StoreExiting = 1UL << 20,
        UseTprShadow = 1UL << 21,
        VirtualNmiPending = 1UL << 22,
        MovDrExiting = 1UL << 23,
        UnconditionalIoExiting = 1UL << 24,
        ActivateIoBitmaps = 1UL << 25,
        MonitorTrapFlag = 1UL << 27,
		ActivateMsrBitmaps = 1UL << 28,
        MonitorExiting = 1UL << 29,
        PauseExiting = 1UL << 30,
        ActivateSecondaryControls = 1UL << 31,
    };

    enum class SecondaryControls : u32 {
        EnableEpt = 1UL << 1,
        EnableRdtscp = 1UL << 3,
        EnableVpid = 1UL << 5,
        UnrestrictedGuest = 1UL << 7,
    	VirtualInterruptDelivery = 1UL << 9,
    	EnableInvpcid = 1UL << 12,
        EnableVmFunctions = 1UL << 13,
    	EnableXsaves = 1ULL << 20,
    	EnableUserWaitPause = 1UL << 26,
    };

    enum class PinBasedControls : u32 {
        ExternalInterruptExiting = 1UL << 0,
        NmiExiting = 1UL << 3,
        VirtualNmi = 1UL << 5,
        ActivateVmxPreemptionTimer = 1UL << 6,
        ProcessPostedInterrupts = 1UL << 7,
    };

    enum class VmExitControls : u32 {
    	SaveDebugControls = 1UL << 2,
        Ia32EMode = 1UL << 9,
        AckIntrOnExit = 1UL << 15,
        SaveGuestPat = 1UL << 18,
        LoadHostPat = 1UL << 19,
    };

    enum class VmEntryControls : u32 {
        Ia32AMode = 1UL << 9,
    	LoadDebugControls = 1UL << 2,
        Smm = 1UL << 10,
        DeactDualMonitor = 1UL << 11,
        LoadGuestPat = 1UL << 14,
    };

    enum class VmExitReason : u32 {
		ExcNmi = 0,
		ExternalInterrupt = 1,
		TripleFault = 2,
		Init = 3,
		Sipi = 4,
		IoSmi = 5,
		OtherSmi = 6,
		PendingVirtIntr = 7,
		PendingVirtNmi = 8,
		TaskSwitch = 9,
		Cpuid = 10,
		Getsec = 11,
		Hlt = 12,
		Invd = 13,
		Invlpg = 14,
		Rdpmc = 15,
		Rdtsc = 16,
		Rsm = 17,
		Vmcall = 18,
		Vmclear = 19,
		Vmlaunch = 20,
		Vmptrld = 21,
		Vmptrst = 22,
		Vmread = 23,
		Vmresume = 24,
		Vmwrite = 25,
		Vmxoff = 26,
		Vmxon = 27,
		CrAccess = 28,
		DrAccess = 29,
		IoInstruction = 30,
		MsrRead = 31,
		MsrWrite = 32,
		InvalidGuestState = 33,
		MsrLoading = 34,
		MwaitInstruction = 36,
		MonitorTrapFlag = 37,
		MonitorInstruction = 39,
		PauseInstruction = 40,
		MceDuringVmentry = 41,
		TprBelowThreshold = 43,
		ApicAccess = 44,
		AccessGdtrOrIdtr = 46,
		AccessLdtrOrTr = 47,
		EptViolation = 48,
		EptMisconfig = 49,
		Invept = 50,
		Rdtscp = 51,
		VmxPreemptionTimerExpired = 52,
		Invvpid = 53,
		Wbinvd = 54,
		Xsetbv = 55,
		ApicWrite = 56,
		Rdrand = 57,
		Invpcid = 58,
		Rdseed = 61,
		PmlFull = 62,
		Xsaves = 63,
		Xrstors = 64,
		Pcommit = 65,
    	
    	Max,
    };
	
	enum class PrivilegeLevel : u32 {
		DplSystem = 0,
		DplUser = 3,
	};
	
	union SegmentAccessRights {
		u32 All;
		struct {
			u32 type : 4;
			u32 descriptorType : 1;
			PrivilegeLevel dpl : 2;
			u32 present : 1;
			u32 reserved0 : 4 = 0;
			u32 available : 1;
			u32 longMode : 1;
			u32 defaultBig : 1;
			u32 granularity : 1;
			u32 unusable : 1 = false;
			u32 reserved1 : 15 = 0;
		} Fields;
    	static_assert(sizeof(Fields) == sizeof(All));
	};
	
	struct SegmentInfo {
		u16 selector;
		SegmentAccessRights accessRights;
		u32 limit;
		u64 base;
	};
	
	enum class ActivityState : u32 {
		Active = 0,
		Hlt = 1,
		Shutdown = 2,
		WaitForSipi = 3,
	};
	
	union InterruptibilityState {
		u32 All;
		struct {
			u32 blockingBySti : 1;
			u32 blockingByMovSs : 1;
			u32 blockingBySmi : 1;
			u32 blockingByNmi : 1;
			u32 enclaveInterruption : 1;
			u32 reserved : 27;
		} Fields;
    	static_assert(sizeof(Fields) == sizeof(All));
	};
	
	enum class InterruptionType : u32 {
		ExternalInterrupt = 0,
		Reserved = 1,
		Nmi = 2,
		HardwareException = 3,
		SoftwareInterrupt = 4,
		PrivilegedSoftwareException = 5,
		SoftwareException = 6,
		Other = 7
	};
	
	union InterruptionInfo {
		u32 All;
		struct {
			u32 vector : 8;
			InterruptionType type : 3;
			u32 deliverErrorCode : 1;
			u32 reserved0 : 1 = 0;
			u32 nestedException : 1 = false;
			u32 reserved : 17 = 0;
			u32 valid : 1;
		} Fields;
    	static_assert(sizeof(Fields) == sizeof(All));
	};
	
	enum class ExceptionVector : u32 {
		DivideError = 0,
		DebugException = 1,
		NmiInterrupt = 2,
		Breakpoint = 3,
		Overflow = 4,
		BoundRangeExceeded = 5,
		InvalidOpcode = 6,
		DeviceNotAvailable = 7,
		DoubleFault = 8,
		CoprocessorSegmentOverrun = 9,
		InvalidTss = 10,
		SegmentNotPresent = 11,
		StackSegmentFault = 12,
		GeneralProtection = 13,
		PageFault = 14,
		Reserved0 = 15,
		MathFault = 16,
		AlignmentCheck = 17,
		MachineCheck = 18,
		SimdException = 19,
		VirtualizationException = 20,
		ControlProtectionException = 21,
	};
	
	union ExitReason {
		u32 All;
		struct {
			VmExitReason basicReason : 16;
			u32 reserved0 : 1 = 0;
			u32 reserved1 : 8 = 0;
			u32 shadowStackBusy : 1;
			u32 busLockAsserted : 1;
			u32 enclaveMode : 1;
			u32 pendingMtfExit : 1;
			u32 exitFromVmxRoot : 1;
			u32 reserved2 : 1;
			u32 entryFailure : 1;
		} Fields;
		static_assert(sizeof(Fields) == sizeof(All));
	};
	
	Expected<PrivilegeLevel> GetGuestCpl();
}

template <> constexpr bool g_isBitmaskEnum<VMCS::PrimaryControls> = true;
template <> constexpr bool g_isBitmaskEnum<VMCS::SecondaryControls> = true;
template <> constexpr bool g_isBitmaskEnum<VMCS::PinBasedControls> = true;
template <> constexpr bool g_isBitmaskEnum<VMCS::VmExitControls> = true;
template <> constexpr bool g_isBitmaskEnum<VMCS::VmEntryControls> = true;
