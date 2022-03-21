#include "Util_x86.h"
#include "../Def/Instruction.h"
#include "../Gen/GenContext_x86.h"

string Util_x86::GetRegName(EReg reg, int size) {
	constexpr const char* GENREG_NAMES_32[REG_COUNT] {
		"EAX", "ECX", "EDX", "EBX", "ESP", "EBP", "ESI", "EDI"
	};

	constexpr const char* GENREG_NAMES_16[REG_COUNT] {
		"AX", "CX", "DX", "BX", "SP", "BP", "SI", "DI"
	};

	constexpr const char* GENREG_NAMES_8[REG_COUNT] {
		"AL", "CL", "DL", "BL", "AH", "CH", "SIL", "DIL"
	};

	switch (size) {
	case 4:
		return GENREG_NAMES_32[(int)reg];
	case 2:
		return GENREG_NAMES_16[(int)reg];
	case 1:
		return GENREG_NAMES_8[(int)reg];
	default:
		return "[UNKNOWN REG (index: " + std::to_string((int)reg) + ", size: " + std::to_string(size) + ")]";
	}
	
}
  
void Util_x86::AppendRelOffset(Instruction& inst, BYTE modRM_Code, BYTE modRM_RM, RegRef regRef) {
	ASSERT(inst.IsValid());
	ASSERT(regRef.reg != REG_INVALID);

	if (regRef.relOffset) {
		AppendManualRelOffset(inst, regRef.reg, modRM_Code, modRM_RM, regRef.relOffset);
	} else {
		inst.Append(
			MAKE_MODRM(
				EModRM_Mod::REG_DIRECT,
				modRM_Code,
				modRM_RM
			)
		);
	}
}

void Util_x86::AppendManualRelOffset(Instruction& inst, EReg reg, BYTE modRM_Code, BYTE modRM_RM, int offset) {
	if (offset == 0 && reg != REG_SP && reg != REG_BP) {
		// Registers other than BP and SP can have shorter instructions for a non-offset reference

		inst.Append(
			MAKE_MODRM(
				EModRM_Mod::ZERO,
				modRM_Code,
				modRM_RM
			)
		);

	} else {
		bool needsMultibyteOffset = offset < CHAR_MIN || offset > CHAR_MAX;

		inst.Append(
			MAKE_MODRM(
				(needsMultibyteOffset ? EModRM_Mod::DISPLACE_16_OR_32 : EModRM_Mod::DISPLACE_8),
				modRM_Code,
				modRM_RM
			)
		);

		if (needsMultibyteOffset) {
			inst.Append<int>(offset);
		} else {
			inst.Append<BYTE>(offset);
		}
	}
}

UINT32 Util_x86::AlignUp(UINT32 val, UINT32 alignMult) {
	return FW::RoundUp(val, alignMult);
}