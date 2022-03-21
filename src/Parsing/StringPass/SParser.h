#pragma once
#include "SToken/SToken.h"

namespace SParser {
	int GetCharType(char c);
	vector<SToken> GetStringTokens(const string& lineStr, int lineNum);
}