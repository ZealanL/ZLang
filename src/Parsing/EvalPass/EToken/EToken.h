#pragma once
#include "ETokenInfos.h"
#include "../../StringPass/SToken/SToken.h"

enum class ETokenType {
	UNKNOWN = -1,
	
	KEYWORD,
	NUMBER,
	IDENTIFIER,
	OPERATOR,
	UTIL_SYMBOL,
};

struct EToken {
	SToken sToken;

	ETokenType type;

	union {
		Keywords::KeywordInfo keywordInfo;
		Numbers::NumberInfo numberInfo;
		Operators::OperatorInfo operatorInfo;
		Symbols::SymbolInfo symbolInfo;
	};

	EToken(SToken sToken, ETokenType type = ETokenType::UNKNOWN) : sToken(sToken), type(type) {}
};