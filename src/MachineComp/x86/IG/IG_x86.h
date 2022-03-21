#pragma once
#include "../Def/Def.h"
#include "../Def/Instruction.h"
#include "../Gen/GenContext_x86.h"

enum {
	LOGIC_INVALID,

	LOGIC_OR,
	LOGIC_AND,
	LOGIC_XOR,

	LOGIC_ADD,
	LOGIC_SUB,

	LOGIC_COUNT
};

enum {
	SHIFT_DIR_LEFT,
	SHIFT_DIR_RIGHT_UNSIGNED,
	SHIFT_DIR_RIGHT_SIGNED,
};

enum {
	MULDIV_MUL_UNSIGNED = 4,
	MULDIV_MUL_SIGNED = 5,

	MULDIV_DIV_UNSIGNED = 6,
	MULDIV_DIV_SIGNED = 7,
};

namespace IG_x86 {
	Instruction MI_MOV_REG_32(RegRef to, RegRef from);
	Instruction MI_PUSH(EReg reg);
	Instruction MI_POP(EReg reg);

	// NOTE: ADD/SUB is faster because INC/DEC sets flags
	Instruction MIG_INC_DEC_32(RegRef reg, bool inc = true);

	Instruction MIG_LOGIC_32(RegRef to, RegRef from, int logicType);
	Instruction MI_MOVSX(EReg toReg, RegRef from, IntSize fromSize);
    Instruction MI_MOV_8(RegRef to, EReg from);
	Instruction MI_MOV_IMM_32(RegRef to, UINT32 val);

	// Regs:
	// EDX:EAX = EAX */ src
	// If multiplying, EDX is the overflow bits
	// If dividing, EDX is the remainder
	Instruction MI_MUL_DIV_REGS_32(RegRef src, int mulDivType);

	// offset: Amount to shift
	// NOTE: If offset is 0, the shift amount will be from the CL register
    Instruction MI_SHIFT_32(RegRef val, BYTE shiftAmount, int shiftDir);
}