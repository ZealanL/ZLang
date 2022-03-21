#pragma once
#include "../Def/Instruction.h"
#include "../../../ZIL/ZIL.h"
#include "../Util/Util_x86.h"

#define REGISTERVARSLOT_INVALID RegisterVarSlot(0, REG_INVALID)
struct RegisterVarSlot {
	EReg reg;
	int size;

	VarId usingVar = VARID_INVALID;
	bool isUsed = false;

	void Expire() {
		usingVar = VARID_INVALID;
		isUsed = false;
	}

	RegisterVarSlot(int size, EReg regIndex, VarId usingVar = VARID_INVALID) : size(size), reg(regIndex), usingVar(usingVar) {}


	RegisterVarSlot() {
		*this = REGISTERVARSLOT_INVALID;
	}

	string ToString() {
		std::stringstream stream;
		stream << "REG[" << (int)reg << "] (" << Util_x86::GetRegName(reg, size) << ")";
		return stream.str();
	}

	bool IsValid() {
		return size > 0 && reg != REG_INVALID;
	}
};

struct StackVarSlot {
	int size, ebpOffset;

	VarId usingVar;
	bool isUsed = false;

	void Expire() {
		usingVar = VARID_INVALID;
		isUsed = false;
	}

	StackVarSlot(int size, int ebpOffset, VarId usingVar = VARID_INVALID) : size(size), ebpOffset(ebpOffset), usingVar(usingVar) {}

	string ToString() {
		std::stringstream stream;
		stream << "STACK[ebp" << (ebpOffset < 0 ? " - " : " + ") << abs(ebpOffset) << "]";
		return stream.str();
	}
};

struct VarSlotWrap_x86 {
	bool isValid;
	bool isStack;
	union {
		RegisterVarSlot* reg;
		StackVarSlot* stack;
	};

	bool IsInUse() {
		return isStack ? stack->isUsed : reg->isUsed;
	}

	void Expire() {
		if (isStack) {
			stack->Expire();
		} else {
			reg->Expire();
		}
	}

	int& GetSize() {
		return isStack ? stack->size : reg->size;
	}

	VarId& GetVarId() {
		return isStack ? stack->usingVar : reg->usingVar;
	}

	VarSlotWrap_x86() : isStack(false), isValid(false) {}

	VarSlotWrap_x86(RegisterVarSlot* regVar) : reg(regVar), isStack(false), isValid(true) {}
	VarSlotWrap_x86(StackVarSlot* stackVar) : stack(stackVar), isStack(true), isValid(true) {}

	bool operator==(VarSlotWrap_x86 other) {
		return reg == other.reg;
	}

	string ToString() {
		if (isStack) {
			return stack->ToString();
		} else {
			return reg->ToString();
		}
	}

	RegRef ToRegRef() {
		if (isStack) {
			return RegRef(REG_BP, stack->ebpOffset);
		} else {
			return RegRef(reg->reg);
		}
	}
};


class GenContext_x86 {
public:
	constexpr static int REGSLOT_COUNT = 6;
private:
	// Array of level numbers for each general register slot in GenContext_x86
	// "Level" = how many "levels" of push/pop a register is at (registers used by variables don't count)
	//	(aka how many times it has been backed up to stack so it can be used for something else)
	int tempRegLevelSlots[REGSLOT_COUNT];

	// What register slots were used
	bool usedRegisterSlots[REGSLOT_COUNT];

	bool finished = false; // Final additions to insts have been made, we are done here
public:
	vector<Instruction> insts;

	// We can use the 6 standard general registers
	// We will NOT use SP or BP for obvious reasons
	RegisterVarSlot regVarSlots[REGSLOT_COUNT] {
		{PTR_x86_SIZE, REG_A},
		{PTR_x86_SIZE, REG_B},
		{PTR_x86_SIZE, REG_C},
		{PTR_x86_SIZE, REG_D},
		{PTR_x86_SIZE, REG_SI},
		{PTR_x86_SIZE, REG_DI},
	};

	int RegToSlotIndex(EReg reg) {
		for (int i = 0; i < REGSLOT_COUNT; i++) {
			if (regVarSlots[i].reg == reg)
				return i;
		}

		return -1;
	}

	EReg SlotIndexToReg(int slotIndex) {
		if (slotIndex > -1 && slotIndex < REGSLOT_COUNT)
			return regVarSlots[slotIndex].reg;
		else
			return REG_INVALID;
	}

	// List datatype such that pointers are consistent
	list<StackVarSlot> stackVarSlots;

	// Either finds or creates the x86 slot for a ZIL variable
	VarSlotWrap_x86 GetVarSlot(VarId id, int size);
	VarSlotWrap_x86 GetVarSlot(VarRef varRef);

	// Makes (or finds unused) x86 slot for some data of a certain size
	VarSlotWrap_x86 MakeVarSlot(int size);

	EReg GetUnusedReg();

	EReg StartTempReg(EReg regWeWant = REG_INVALID);

	// NOTE: Does nothing is reg is invalid
	void EndTempReg(EReg reg);

	vector<Instruction> Finish();
};