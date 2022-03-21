#pragma once
#include "../../../CoreTypes.h"
#include "../../../CoreTypes.h"

class SToken {
public:
	// Can safely assume this is not empty
	ZLT_Str str;

	enum {
		CHARTYPE_INVALID,
		CHARTYPE_ALNUM,
		CHARTYPE_SYMBOL, // any non-blank character that isnt alphanumeric, will NOT be com
		CHARTYPE_OP_SYMBOL, // +-/*% etc, can be combined into a single token
		CHARTYPE_BLANK, // Can also be a newline
	};
	int charType;

	int lineNum, charNum;
	SToken(ZLT_Str& str, int charType, int lineNum, int charNum) : str(str), charType(charType), lineNum(lineNum), charNum(charNum) {
		ASSERT(str.length > 0); // Should never be the case
	}
};
