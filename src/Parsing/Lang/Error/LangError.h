#pragma once
#include "LangErrorDef.h"

struct LangError {
	const LangErrorDef* errorDef;
	int lineNum, charNum;
	string description; // Per-instance description if needed

	LangError(const LangErrorDef* errorDef, int lineNum = -1, int charNum = -1) : errorDef(errorDef), lineNum(lineNum), charNum(charNum) {}
};