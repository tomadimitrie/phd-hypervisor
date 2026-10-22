#pragma once

#include "Vcpu.hpp"
#include "Common/Utils.hpp"

extern "C" {
	NTSTATUS AsmLaunchOneCpu(Vcpu& Target);
	
	void AsmPopState();
	
#define MAKE_REG_GETTER(reg) extern "C" u64 Get##reg##Value(void)

	MAKE_REG_GETTER(RAX);
	MAKE_REG_GETTER(RBX);
	MAKE_REG_GETTER(RCX);
	MAKE_REG_GETTER(RDX);
	MAKE_REG_GETTER(RSI);
	MAKE_REG_GETTER(RDI);
	MAKE_REG_GETTER(RBP);
	MAKE_REG_GETTER(RSP);
	MAKE_REG_GETTER(R8);
	MAKE_REG_GETTER(R9);
	MAKE_REG_GETTER(R10);
	MAKE_REG_GETTER(R11);
	MAKE_REG_GETTER(R12);
	MAKE_REG_GETTER(R13);
	MAKE_REG_GETTER(R14);
	MAKE_REG_GETTER(R15);
	MAKE_REG_GETTER(CS);
	MAKE_REG_GETTER(DS);
	MAKE_REG_GETTER(ES);
	MAKE_REG_GETTER(SS);
	MAKE_REG_GETTER(FS);
	MAKE_REG_GETTER(GS);

	u64 GetGdtBase();
	u16 GetGdtLimit();
	u64 GetLdtr();
	u64 GetTr();
	u64 GetIdtBase();
	u16 GetIdtLimit();

	void AsmVmExitHandler();
}
