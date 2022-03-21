#include "GenContext_x86.h"
#include "../IG/IG_x86.h"

VarSlotWrap_x86 GenContext_x86::GetVarSlot(VarRef varRef) {
	return GetVarSlot(varRef.id, PTR_x86_SIZE);
}

VarSlotWrap_x86 GenContext_x86::GetVarSlot(VarId id, int size) {
	ASSERT(!finished);

	// Try to find it in stack
	for (StackVarSlot& stackSlot : stackVarSlots)
		if (stackSlot.usingVar == id && stackSlot.isUsed)
			return VarSlotWrap_x86(&stackSlot);

	// Not found, create new
	VarSlotWrap_x86 newSlot = MakeVarSlot(size);
	newSlot.GetVarId() = id;
	return newSlot;
}

EReg GenContext_x86::GetUnusedReg() {
	for (int i = 0; i < REGSLOT_COUNT; i++)
		if (!regVarSlots[i].isUsed && !tempRegLevelSlots[i])
			return SlotIndexToReg(i);

	return REG_INVALID;
}

EReg GenContext_x86::StartTempReg(EReg regWeWant) {
	bool wantsSpecificReg = regWeWant != REG_INVALID;
	int wantedRegSlotIndex = RegToSlotIndex(regWeWant);

	int bestRegSlotIdx;

	if (wantsSpecificReg) {
		bestRegSlotIdx = wantedRegSlotIndex;
	} else {
		// Find slot with lowest level
		int lowestLevel = tempRegLevelSlots[0];
		bestRegSlotIdx = 0;
		for (int i = 1; i < REGSLOT_COUNT; i++) {
			int level = tempRegLevelSlots[i];
			if (level < lowestLevel) {
				lowestLevel = level;
				bestRegSlotIdx = i;
			}
		}
	}
	
	EReg reg = SlotIndexToReg(bestRegSlotIdx);

	if (tempRegLevelSlots[bestRegSlotIdx] > 0)
		insts.push_back(IG_x86::MI_PUSH(reg));

	tempRegLevelSlots[bestRegSlotIdx]++;
	usedRegisterSlots[bestRegSlotIdx] = true;

	return reg;
}

void GenContext_x86::EndTempReg(EReg reg) {
	if (reg == REG_INVALID)
		return;

	int slotIndex = RegToSlotIndex(reg);
	if (slotIndex == -1)
		return;

	tempRegLevelSlots[slotIndex]--;

	if (tempRegLevelSlots[slotIndex] > 0)
		insts.push_back(IG_x86::MI_POP(reg));
}

VarSlotWrap_x86 GenContext_x86::MakeVarSlot(int size) {
	ASSERT(!finished);
	ASSERT(size >= 0);

	// Try to find space in stack
	for (StackVarSlot& stackSlot : stackVarSlots) {
		if (stackSlot.size >= size && !stackSlot.isUsed) {
			stackSlot.isUsed = true;
			return VarSlotWrap_x86(&stackSlot);
		}
	}

	// No unused slots found, create new slot stack
	int newStackOffset;
	if (stackVarSlots.empty()) {
		newStackOffset = PTR_x86_SIZE;
	} else {
		StackVarSlot& last = stackVarSlots.back();
		newStackOffset = last.ebpOffset + last.size;
	}

	StackVarSlot newStackSlot = StackVarSlot(size, newStackOffset);
	newStackSlot.isUsed = true;
	stackVarSlots.push_back(newStackSlot);

	return VarSlotWrap_x86(&stackVarSlots.back());
}

vector<Instruction> GenContext_x86::Finish() {
	// Add beginning/end pushes and pops
	for (int i = 0; i < REGSLOT_COUNT; i++) {
		if (usedRegisterSlots[i]) {
			auto reg = SlotIndexToReg(i);
			insts.emplace(insts.begin(), IG_x86::MI_PUSH(reg));
			insts.push_back(IG_x86::MI_POP(reg));
		}
	}	

	finished = true;
	return insts;
}