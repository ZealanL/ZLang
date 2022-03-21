#include "Gen_x86.h"

void Gen_x86::MakeVarMOV(GenContext_x86& ctx, VarRef to, VarRef from, VarSlotWrap_x86 slotOverride_to, VarSlotWrap_x86 slotOverride_from) {
	ASSERT(to.actualSize >= from.actualSize);
	int movDataSize = Util_x86::AlignUp(to.actualSize);
	ASSERT(movDataSize > 0);
	int movBlocksNeeded = movDataSize / 4;

	GenResult& result = ctx.insts;

	auto toSlot = slotOverride_to.isValid ? slotOverride_to : ctx.GetVarSlot(to);
	auto fromSlot = slotOverride_from.isValid ? slotOverride_from : ctx.GetVarSlot(from);

	/* PROBLEM SOLVING
	*
	* GOAL: mov [eax], [ebx]
	*
	* SOLUTION:
	*	mov tempReg, [ebx]
	*	mov [eax], tempReg
	*
	* ////////////////////////////////
	*
	* GOAL: mov eax, [[ebp + 4] + 8]
	*
	* SOLUTION:
	*	mov tempReg, [ebp+4]
	*	mov eax, [tempReg]
	*
	* /////////////////////////////////////////
	*
	* GOAL: [[ebp + 8] + 8], [[ebp + 4] + 8]
	*
	* BAD SOLUTION:
	*	mov tempReg1, [ebp+8]
	*	mov tempReg2, [ebp+4]
	*	mov tempReg3, [tempReg2 + 8]
	*	mov [tempReg1], tempReg3
	*
	* BETTER SOLUTION:
	*	mov tempReg1, [ebp+8]
	*	mov tempReg2, [ebp+4]
	*	mov tempReg2, [tempReg2 + 8] // Reuse tempReg2
	*	mov [tempReg1], tempReg2
	*/

	// If we resolve a double-reference, this is the reg it will be in
	RegisterVarSlot tempRefRegSlot_to, tempRefRegSlot_from;

	FixDoubleRelIfNeeded(ctx, to, toSlot, tempRefRegSlot_to);
	FixDoubleRelIfNeeded(ctx, from, fromSlot, tempRefRegSlot_from);

	RegRef toRef = MAKE_REGREF(to, toSlot), fromRef = MAKE_REGREF(from, fromSlot);

	if (toRef.isRel && fromRef.isRel) {
		// Single mov instruction cannot have 2 references, use 2 instructions

		EReg tempReg;
		if (movBlocksNeeded == 1 && tempRefRegSlot_to.reg != REG_INVALID) {
			// We used a temporary reg for fromSlot, we can reuse this
			tempReg = tempRefRegSlot_to.reg;
		} else {
			tempReg = ctx.StartTempReg();
		}

		for (int i = 0; i < movDataSize; i += 4) {
			RegRef curFromRef = fromRef;
			RegRef curToRef = toRef;
			curFromRef.relOffset += i;
			curToRef.relOffset += i;

			result.push_back(
				IG_x86::MI_MOV_REG_32(
					RegRef(tempReg),
					curFromRef
				)
			);

			result.push_back(
				IG_x86::MI_MOV_REG_32(
					curToRef,
					RegRef(tempReg)
				)
			);
		}

		ctx.EndTempReg(tempReg);

	} else {

		for (int i = 0; i < movDataSize; i += 4) {
			RegRef curFromRef = fromRef;
			RegRef curToRef = toRef;
			curFromRef.relOffset += i;
			curToRef.relOffset += i;

			result.push_back(
				IG_x86::MI_MOV_REG_32(
					curToRef,
					curFromRef
				)
			);
		}
	}

	ctx.EndTempReg(tempRefRegSlot_to.reg);
	ctx.EndTempReg(tempRefRegSlot_from.reg);
}

bool Gen_x86::FixDoubleRelIfNeeded(GenContext_x86& ctx, VarRef varRef, VarSlotWrap_x86& slotWrap, RegisterVarSlot& tempRegSlotOut) {
	if (!slotWrap.isValid)
		return false;

	if (varRef.isRel & slotWrap.isStack) {
		tempRegSlotOut.reg = ctx.StartTempReg();
		ctx.insts.push_back(IG_x86::MI_MOV_REG_32(tempRegSlotOut.reg, slotWrap.ToRegRef()));
		tempRegSlotOut.size = PTR_x86_SIZE;

		slotWrap = &tempRegSlotOut;
		return true;
	} else {
		return false;
	}
}