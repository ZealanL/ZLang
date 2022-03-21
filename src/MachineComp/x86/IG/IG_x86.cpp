#include "IG_x86.h"


FINLINE Instruction MakeTwoVarInst_LR(RegRef a, RegRef b, OpCode left, OpCode right) {

	if (a.isRel && b.isRel)
		return INSTRUCTION_INVALID; // Not possible in a single instruction

	if (!a.isRel && !b.isRel) {
		// Simple reg,reg
		return Instruction{ {
			left, MAKE_MODRM(EModRM_Mod::REG_DIRECT, b.reg, a.reg)
		}, "MakeTwoVarInst_LR: Simple reg,reg" };
	} else {
		RegRef rel = a.isRel ? a : b;
		RegRef nonRel = a.isRel ? b : a;

		Instruction inst = { { (rel == a) ? left : right}, "MakeTwoVarInst_LR: Normal rel+nonrel" };

		Util_x86::AppendRelOffset(inst, nonRel.reg, rel.reg, rel);
		return inst;
	}
}

FINLINE Instruction MakeIncOrDec(RegRef ref, bool isInc) {
	if (ref.isRel) {
		Instruction inst = { { OP_FF },  "MakeIncOrDec: rel" };

		int code = isInc ? 0 : 1;

		if (ref.relOffset == 0) {
			inst.Append(
				MAKE_MODRM(EModRM_Mod::ZERO, code, ref.reg)
			);
		} else {
			Util_x86::AppendRelOffset(inst, code, ref.reg, ref);
		}

		return inst;
	} else {
		return { { (isInc ? OP_INC_REG_BASE : OP_DEC_REG_BASE) + (int)ref.reg }, "MakeIncOrDec: nonrel" };
	}
}

#pragma region Implementations
Instruction IG_x86::MI_MOV_REG_32(RegRef to, RegRef from) {
	return MakeTwoVarInst_LR(to, from, OP_MOV_32_L, OP_MOV_32_R);
}

Instruction IG_x86::MI_PUSH(EReg reg) {
	return { { OP_PUSH_REG_BASE + reg }, "MI_PUSH" };
}

Instruction IG_x86::MI_POP(EReg reg) {
	return{ { OP_PUSH_REG_BASE + reg }, "MI_POP" };
}

Instruction IG_x86::MIG_INC_DEC_32(RegRef ref, bool inc) {
	return MakeIncOrDec(ref, inc);
}

Instruction IG_x86::MIG_LOGIC_32(RegRef to, RegRef from, int logicType) {


	switch (logicType) {
	case LOGIC_AND:
		return MakeTwoVarInst_LR(to, from, OP_AND_32_L, OP_AND_32_R);
	case LOGIC_OR:
		return MakeTwoVarInst_LR(to, from, OP_OR_32_L, OP_OR_32_R);
	case LOGIC_XOR:
		return MakeTwoVarInst_LR(to, from, OP_XOR_32_L, OP_XOR_32_R);
	case LOGIC_ADD:
		return MakeTwoVarInst_LR(to, from, OP_ADD_32_L, OP_ADD_32_R);
	case LOGIC_SUB:
		return MakeTwoVarInst_LR(to, from, OP_SUB_32_L, OP_SUB_32_L);
	default:
		return INSTRUCTION_INVALID;
	}
}

Instruction IG_x86::MI_MOVSX(EReg toReg, RegRef from, IntSize fromSize) {
	ASSERT(fromSize == 8 || fromSize == 16);

	UINT32 opCode = (fromSize == 8) ? OP2_MOVSX_REG_8 : OP2_MOVSX_REG_16;

	Instruction inst = { { opCode }, "MI_MOVSX" };
	Util_x86::AppendRelOffset(inst, toReg, from.reg, from);
	return inst;
}

Instruction IG_x86::MI_MOV_8(RegRef to, EReg from) { 
	return MakeTwoVarInst_LR(to, from, OP_MOV_8, OP_MOV_8);
}

Instruction IG_x86::MI_MOV_IMM_32(RegRef to, UINT32 val) {
	if (to.isRel) {
		Instruction inst = { { OP_MOV_IMM_32 }, "MI_MOV_IMM_32: rel" };
		Util_x86::AppendRelOffset(inst, 0, to.reg, to);
		inst.Append<UINT32>(val);
		return inst;
	} else {
		Instruction inst = { { OP_MOV_REG_IMM_32_BASE + to.reg }, "MI_MOV_IMM_32: nonrel" };
		inst.Append<UINT32>(val);
		return inst;
	}
}

Instruction IG_x86::MI_MUL_DIV_REGS_32(RegRef src, int mulDivType) {
	Instruction inst = { { OP_MUL_DIV_REGS_32 }, "MI_MUL_DIV_REGS_32" };
	Util_x86::AppendRelOffset(inst, mulDivType, src.reg, src);
	return inst;
}

Instruction IG_x86::MI_SHIFT_32(RegRef val, BYTE shiftAmount, int shiftDir) {

	BYTE modCode;
	switch (shiftDir) {
	case SHIFT_DIR_LEFT:
		modCode = 4;
		break;
	case SHIFT_DIR_RIGHT_SIGNED:
		modCode = 7;
		break;
	case SHIFT_DIR_RIGHT_UNSIGNED:
		modCode = 5;
		break;
	default:
		modCode = 0;
		ASSERT(false); // Invalid shift direction
	}

	Instruction inst("MI_SHIFT_32");
	bool shiftWithCL = shiftAmount == 0;
	if (shiftWithCL) {
		inst.Append(OP_SHIFT_WITH_CL_32);
	} else {
		bool shiftLeft = shiftAmount > 0;
		int absOffset = abs(shiftAmount);
		bool multiShift = absOffset > 1;

	    inst.Append( multiShift ? OP_SHIFT_IMM_MULTI_32 : OP_SHIFT_IMM_SINGLE_32 );

		if (multiShift) {
			inst.Append<BYTE>(shiftAmount);
		}
	}

	if (val.isRel) {
		Util_x86::AppendRelOffset(inst, modCode, val.reg, val);
	} else {
		inst.Append(MAKE_MODRM(EModRM_Mod::REG_DIRECT, modCode, val.reg));
	}

	

	return inst;
}

#pragma endregion