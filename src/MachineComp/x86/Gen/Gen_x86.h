#pragma once
#include "GenContext_x86.h"
#include "../../../ZIL/Instructions/ZILIDef.h"
#include "../IG/IG_x86.h"

typedef vector<Instruction> GenResult;

#define GENFUNC(t) inline void Gen_##t(GenContext_x86& ctx, ZILI_##t& zili)

// RegRef creation helper with VarRef and VarSlotWrap_x86
#define MAKE_REGREF(varRef, slot) slot.isStack ? slot.ToRegRef() : RegRef(slot.reg->reg, varRef.isRel, varRef.relOffset)

namespace Gen_x86 {
	void MakeVarMOV(GenContext_x86& ctx, VarRef to, VarRef from, VarSlotWrap_x86 slotOverride_to = {}, VarSlotWrap_x86 slotOverride_from = {});

	// If the provided var ref+slot is a double relative (aka a relative var ref AND stack slot), this will:
	// - Use ctx.StartTempReg() for a temporary register
	// - Add a MOV to the ctx to move ptr data from stack to the new temporary reg
	// - Sets tempRegSlotOut to the temporary register
	// - Sets slotWrap to the tempRegSlotOut
	// Returns true if it was a double relative
	// NOTE: Caller must end the temporary register themselves
	bool FixDoubleRelIfNeeded(GenContext_x86& ctx, VarRef varRef, VarSlotWrap_x86& slotWrap, RegisterVarSlot& tempRegSlotOut);

	GENFUNC(Set) {
		MakeVarMOV(ctx, zili.destVar, zili.srcVar);
	}

	GENFUNC(SetVal) {
		auto to = zili.var;
		auto dataSize = zili.valBytes.Size();

		ASSERT(dataSize > 0 && dataSize >= 0);

		int movDataSize = Util_x86::AlignUp(dataSize);

		auto toSlot = ctx.GetVarSlot(to);
		
		RegisterVarSlot tempRegSlot;
		FixDoubleRelIfNeeded(ctx, to, toSlot, tempRegSlot);

		// Fix if double relative
		if (toSlot.isStack && to.isRel) {
			
			tempRegSlot.reg = ctx.StartTempReg();
			ctx.insts.push_back(IG_x86::MI_MOV_REG_32(tempRegSlot.reg, toSlot.ToRegRef()));
			toSlot = &tempRegSlot;
		}

		for (int i = 0; i < movDataSize; i += PTR_x86_SIZE) {

			int data = zili.valBytes.Read<int>(i, 0);

			RegRef curTo = toSlot.isStack ? toSlot.ToRegRef() : RegRef(toSlot.reg->reg, to.isRel, to.relOffset);
			curTo.relOffset += i;

			ctx.insts.push_back(IG_x86::MI_MOV_IMM_32(curTo, data));
		}

		ctx.EndTempReg(tempRegSlot.reg);
	}

	GENFUNC(IntMath) {

		// Operational value is stored in a variable rather than raw data
		bool isOpValVar = zili.opVal.isVar;

		// Our output var is the same as our input var
		bool outputIsInput = zili.outputVar == zili.inputVar;

		// Sizes must match
		ASSERT(zili.outputVar.actualSize == zili.inputVar.actualSize);

		// TODO: Only 32-bit int math is currently supported
		ASSERT(zili.outputVar.actualSize == PTR_x86_SIZE);
		ASSERT(zili.inputVar.actualSize == PTR_x86_SIZE);
		ASSERT(zili.opVal.GetSize() == PTR_x86_SIZE);

		// NOTE: Could be signed or unsigned
		UINT32 opVal = zili.opVal.isVar ? 0 : zili.opVal.byteData.Read<UINT32>();

		VarSlotWrap_x86 slot_opValVar, slot_output, slot_input;
		RegisterVarSlot tempRegSlot_opValVar, tempRegSlot_output, tempRegSlot_input;

		slot_input = ctx.GetVarSlot(zili.inputVar);
		FixDoubleRelIfNeeded(ctx, zili.inputVar, slot_input, tempRegSlot_input);

		// Need to copy input to output
		if (!outputIsInput) {
			slot_output = ctx.GetVarSlot(zili.outputVar);
			FixDoubleRelIfNeeded(ctx, zili.outputVar, slot_output, tempRegSlot_output);
		}

		if (isOpValVar) {
			slot_opValVar = ctx.GetVarSlot(zili.opVal.varRef);
			FixDoubleRelIfNeeded(ctx, zili.opVal.varRef, slot_opValVar, tempRegSlot_opValVar);
		}

		DLOG("IntMath:");
		if (zili.opVal.isVar) {
			DLOG(" - opVal: " << zili.opVal.ToString() << " : " << slot_opValVar.ToString());
		} else {
			DLOG(" - opVal: " << zili.opVal.byteData.ToString());
		}
		DLOG(" - output: " << zili.outputVar.ToString() << " : " << slot_output.ToString());
		if (!outputIsInput)
			DLOG(" - input: " << zili.inputVar.ToString() << " : " << slot_input.ToString());

		if (!outputIsInput) {
			MakeVarMOV(ctx, zili.outputVar, zili.inputVar, slot_output, slot_input);
		}

		// Two relatives will be used, multiple instructions will be needed
		bool twoRels = (slot_output.isStack || zili.outputVar.isRel) && isOpValVar && (slot_opValVar.isStack || zili.opVal.varRef.isRel);
		
		int logicOp = LOGIC_INVALID;
		switch (zili.operation) {
		case ZILI_MATH_OP::ADD:
			logicOp = LOGIC_ADD;
			break;
		case ZILI_MATH_OP::SUB:
			logicOp = LOGIC_SUB;
			break;
		case ZILI_MATH_OP::OR:
			logicOp = LOGIC_OR;
			break;
		case ZILI_MATH_OP::AND:
			logicOp = LOGIC_AND;
			break;
		}
		
		RegRef outputRegRef = MAKE_REGREF(zili.outputVar, slot_output);

		// Can IG_x86::MIG_LOGIC_32 can be used?
		if (logicOp != LOGIC_INVALID) {
			EReg tempOutputReg = REG_INVALID;
			RegRef curOutputRegRef = outputRegRef;
			if (twoRels) {
				tempOutputReg = ctx.StartTempReg();
				ctx.insts.push_back(
					IG_x86::MI_MOV_REG_32(RegRef(tempOutputReg), outputRegRef)
				);
				curOutputRegRef = RegRef(tempOutputReg);
			}

			if (isOpValVar) {
				ctx.insts.push_back(
					IG_x86::MIG_LOGIC_32(curOutputRegRef, MAKE_REGREF(zili.opVal.varRef, slot_opValVar), logicOp)
				);
			} else {
				ASSERT(false);
			}

			if (twoRels) {
				ctx.insts.push_back(
					IG_x86::MI_MOV_REG_32(outputRegRef, RegRef(tempOutputReg))
				);

				ctx.EndTempReg(tempOutputReg);
			}

		} else if (zili.operation == ZILI_MATH_OP::MULT || zili.operation == ZILI_MATH_OP::DIV) {
			
			int mulDivType =
				(zili.operation == ZILI_MATH_OP::DIV) ?
				(zili.isSigned ? MULDIV_DIV_SIGNED : MULDIV_DIV_UNSIGNED)
				: (zili.isSigned ? MULDIV_MUL_SIGNED : MULDIV_MUL_UNSIGNED);
			
			
			// If possible, use signed multiplication instead of unsigned (it doesn't matter so long as nothing needs to be sign-extended or overflows)
			if (mulDivType == MULDIV_MUL_UNSIGNED && (zili.inputVar.actualSize == zili.outputVar.actualSize == zili.opVal.GetSize()))
				mulDivType = MULDIV_MUL_SIGNED;

			// EAX and EDX registers will be used against our will
			bool oneOperandForm = mulDivType != MULDIV_MUL_SIGNED;

			RegRef outputRegRef = MAKE_REGREF(zili.outputVar, slot_output);
			EReg outputReg = outputRegRef.reg;
			bool tempOutputRegUsed = false;
			if (outputRegRef.isRel || (oneOperandForm && outputRegRef.reg != REG_A)) {
				outputReg = ctx.StartTempReg(oneOperandForm ? REG_A : REG_INVALID);;
				tempOutputRegUsed = true;
			}

			if (oneOperandForm)
				ctx.StartTempReg(REG_D);

			if (oneOperandForm) {
				if (isOpValVar) {
					ASSERT(false);
				} else {
					ASSERT(false);
				}
			} else {
				if (mulDivType == MULDIV_MUL_SIGNED) {
					if (isOpValVar) {
						ASSERT(false);
					} else {
						ASSERT(zili.opVal.byteData.Size() == 1 || zili.opVal.byteData.Size() == 4); // Must be int8 int32

						bool needsMultiByteImm = zili.opVal.byteData.Size() > 1;
						Instruction inst = { {
							needsMultiByteImm ? OP_IMUL_32_IMM_32 : OP_IMUL_32_IMM_8,
							MAKE_MODRM(EModRM_Mod::REG_DIRECT, outputReg, outputReg)
						}, "IntMath_MULDIV_MUL_SIGNED_IMM" };

						inst.AppendBytes(zili.opVal.byteData);

						ctx.insts.push_back(inst);
					}
				} else {
					ASSERT(false);
				}
			}

			if (tempOutputRegUsed) {
				ctx.insts.push_back(IG_x86::MI_MOV_REG_32(outputRegRef, outputReg));
				ctx.EndTempReg(outputReg);
			}

			if (oneOperandForm)
				ctx.EndTempReg(REG_D);
			
		} else {
			ASSERT(false);// Unimplemented
		}

		ctx.EndTempReg(tempRegSlot_opValVar.reg);
		ctx.EndTempReg(tempRegSlot_output.reg);
		ctx.EndTempReg(tempRegSlot_input.reg);
	}
}

#undef GENFUNC