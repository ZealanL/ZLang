#pragma once
#include "../../../CoreTypes.h"

struct LangErrorDef {
	UINT16 id;
	const char* errorStr;
};

#define LANG_ERROR_PLACEHOLDER_TOKENNAME "$n"

#define MAKE(id, name, str) constexpr LangErrorDef LANG_ERROR_##name = LangErrorDef{id, #str}
#pragma region Definitions
MAKE(1, INVALID_TOKEN, "invalid token $n");
MAKE(2, UNKNOWN_IDENTIFIER, "unknown identifier");
MAKE(3, UNEXPECTED, "unexpected $n");
MAKE(4, ALREADY_EXISTS, "$n already exists");

MAKE(20, MISSING_CLOSING_PAREN, "expected closing parenthesis");
MAKE(21, EXTRA_CLOSING_PAREN, "extra closing bracket has no opening parenthesis");

MAKE(100, INVALID_NUMBER, "invalid number $n")


#pragma endregion
#undef MAKE
