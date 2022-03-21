#pragma once
#include "../../../CoreTypes.h"

// USEFUL INFO LINKS:
// https://paul.bone.id.au/blog/2018/09/26/more-x86-addressing/
// https://www.felixcloutier.com/x86

// 32-bit
typedef UINT32 PTR_x86;
#define PTR_x86_SIZE sizeof(PTR_x86)

// Size of an integer in bits (8, 16, 32, etc.)
// Only goes to 128 for now
typedef BYTE IntSize;

#define OP_LR(name, base) \
	OP_##name##_L = base, \
	OP_##name##_R = (base + 2)

enum OpCode : UINT16 {

	// OP_  = Standard single-byte opcode
	// OP2_ = 2-byte opcode

	OP_LR(ADD_32, 0x01),
	OP_LR(SUB_32, 0x29),
	OP_LR(XOR_32, 0x31),
	OP_LR(OR_32, 0x09),
	OP_LR(AND_32, 0x21),

	OP2_IMUL_32 = '\x0F\xAF', // doubleword register <- doubleword register : r/m32

	OP2_MOVSX_REG_8 = '\x0F\xBE', // doubleword register <- signed byte : r/m8
	OP2_MOVSX_REG_16 = '\x0F\xBF', // doubleword register <- signed word : r/m16

	OP_INC_REG_BASE = 0x40,
	OP_DEC_REG_BASE = 0x48,

	OP_PUSH_REG_BASE = 0x50,
	OP_POP_REG_BASE = 0x58,

	OP_IMUL_32_IMM_32 = 0x69, // doubleword register <- r/m32 : immediate doubleword
	OP_IMUL_32_IMM_8 = 0x6B,  // doubleword register <- r/m32 : sign-extended immediate byte

	OP_MOV_8 = 0x88,

	OP_MOV_REG_IMM_32_BASE = 0xB8,
	OP_MOV_IMM_32 = 0xC7,

	OP_LR(MOV_32, 0x89),

	OP_LEA = 0x8D,

	OP_SHIFT_IMM_MULTI_32 = 0xC1,	
	OP_SHIFT_IMM_SINGLE_32 = 0xD1,
	OP_SHIFT_WITH_CL_32 = 0xD3,

	OP_INC_OR_DEC_BYTE = 0xFE,


	// Multiply/div EDX:EAX by r/m32, with result stored in EAX <- Quotient/Result, EDX <- Remainder/Overflow
	// F7 /4 = mul unsigned
	// F7 /5 = mul signed
	// F7 /6 = div unsigned
	// F7 /7 = div signed
	OP_MUL_DIV_REGS_32 = 0xF7, 

	OP_FF = 0xFF,
};

#undef OP_LR

enum EReg : BYTE {
	REG_INVALID = -1,

	REG_A, REG_C, REG_D, REG_B, // Primary 4 (EAX, ECX, EDX, EBX)
	REG_SP, REG_BP,		// Stack pointer and frame pointer (ESP, EBP)
	REG_SI, REG_DI,		// Extra i-regs (ESI, EDI)

	REG_COUNT,

	// Extras
	REG_AL = REG_A,
	REG_AH = REG_SP,
	REG_CL = REG_A,
	REG_CH = REG_BP,
};

#pragma region ModRM
// 2 bits
enum class EModRM_Mod {
	ZERO, // Register Indirect Addressing Mode or SIB with no displacement or Displacement Only
	DISPLACE_8,
	DISPLACE_16_OR_32,
	REG_DIRECT,

	COUNT
};

// 3 bits
enum class EModRM_RM {
	/* 0x00 */ NONE,   // No Size
	/* 0x01 */ OTHER_TABLE,   // Size Specified in Another Table

	/* 0x02 */ BYTE,   // Byte, regardless of operand-size
	/* 0x03 */ WORD,   // Word, regardless on operand-size
	/* 0x04 */ DWORD,  // Dword, regardless of operand-size

	/* 0x05 */ WORD_OR_DWORD_FROM_OP_SIZE,   // Word or dword, depending on operand-size
	/* 0x06 */ ANY_FROM_OP_SIZE,   // Byte, word, or dword depending on operand-size

	/* 0x07 */ DWORD_OR_QWORD_FROM_OP_SIZE,   // Two word or two dword operands in memory, depending on operand-size

	/* 0x08 */ POINTER,   // 32-bit pointer

	COUNT
};

#pragma endregion

enum class ESIB_Scale {
	NONE,
	ONE,
	TWO,
	FOUR,
	EIGHT,

	COUNT
};


#define XMM_REGISTER_COUNT_32 8
#define XMM_REGISTER_COUNT_64 16

#define MAKE_MODRM(mod, code, rm) (BYTE((int)rm | ((int)code<<3) | ((int)mod<<6)))