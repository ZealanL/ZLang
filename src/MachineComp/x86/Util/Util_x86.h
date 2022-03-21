#pragma once
#include "../Def/Def.h"
#include "../Def/Instruction.h"


namespace Util_x86 {
	string GetRegName(EReg reg, int size = PTR_x86_SIZE);
    void AppendRelOffset(Instruction& inst, BYTE modRM_Code, BYTE modRM_RM, RegRef regRef);
    void AppendManualRelOffset(Instruction& inst, EReg reg, BYTE modRM_Code, BYTE modRM_RM, int offset);
    UINT32 AlignUp(UINT32 val, UINT32 alignMult = PTR_x86_SIZE);
}