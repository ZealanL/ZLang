#include "EParser.h"

#include "../StringPass/SParser.h"

typedef bool(*ETokenTrySetInfoFn)(EToken& token);

// Each of the functions in this namespace try to evaluate a token under a certain type
// If they return true, the token is under that type (although it is not guarenteed to be valid)
namespace TrySetInfoFuncs {
	bool Num(EToken& token) {
		ZLT_Str strData = token.sToken.str.ToLower();

		// Can't be a number
		if (!isdigit(strData[0]))
			return false;

		token.type = ETokenType::NUMBER;
		Numbers::NumberInfo& numberInfo = token.numberInfo;
		numberInfo.type = Numbers::TYPE_INVALID;

		bool isHex = strData.StartsWith("0x");
		bool isBinary = !isHex && strData.StartsWith("0b");

		bool hasPrefix = isHex || isBinary;

		bool hasDecimal = strData.HasChar('.');

		char suffixChar = strData[strData.length - 1];
		bool hasSuffix = !isdigit(suffixChar);

		if (hasPrefix) {
			if (strData.length < 3)
				return false; // Invalid, no chars after prefix

			if (hasDecimal)
				return false; // Integers can't have decimals
		}

		bool isFloat = !hasPrefix && (hasSuffix || hasDecimal);
		if (isFloat) {
			bool foundDecimal = false;
			for (int i = 0; i < strData.length - 1; i++) {
				char c = strData[i];
				if (c == '.') {
					if (!foundDecimal) {
						foundDecimal = true;
					} else {
						return false; // Invalid, has multiple decimals
					}
				} else if (!isdigit(c))
					return false; // Invalid, all chars (except for last) must be digit or dot
			}

			bool isFloat64 = false;
			if (hasSuffix) {
				if (suffixChar == 'f') {
					isFloat64 = false;
				} else if (suffixChar == 'd') {
					isFloat64 = true;
				} else {
					return false; // Invalid suffix
				}
			}

			double val = 0;
			try {
				string strDataTemp = strData;

				if (hasSuffix) {
					strDataTemp.erase(strDataTemp.size() - 1);
				}
				val = std::stod(strDataTemp);
			} catch (std::exception e) {
				return false; // Invalid number
			}

			if (isFloat64) {
				numberInfo.type = Numbers::TYPE_FLOAT64;
				numberInfo.v_f64 = val;
			} else {
				numberInfo.type = Numbers::TYPE_FLOAT32;
				numberInfo.v_f32 = val;
			}
		} else {
			UINT64 intBuffer = 0;

			if (hasPrefix) {

				int len = strData.length - 2;
				for (int i = 0; i < len; i++) {
					char c = strData[i + 2];
					UINT64 curPlace = len - i - 1;

					if (isBinary) {
						switch (c) {
						case '0':
							break;
						case '1':
							intBuffer += (1 << curPlace);
							break;
						default:
							// Invalid binary number
							return false;
						}
					} else if (isHex) {

						bool isDigit = isdigit(c);

						if (!(isDigit || (c >= 'a' && c <= 'f')))
							return false; // Invalid hex char

						BYTE octalVal = isDigit ? c - '0' : c - 'a' + 0xA;

						intBuffer += octalVal * (1 << curPlace * 4);
					} else {
						SHOULD_BE_IMPOSSIBLE;
					}
				}
			} else {
				// Standard integer

				UINT64 curPlacePower = 1; // Multiplied by 10 each iteration, the value of the current digit's "place" (i.e. ones, tens, hundreds, etc.)
				for (int i = strData.length - 1; i >= 0; i--) {
					char c = strData[i];

					if (!isdigit(c))
						return false; // Standard integers must be entirely numbers

					intBuffer += curPlacePower * (c - '0');

					curPlacePower *= 10;
				}
			}

			if (intBuffer > UINT32_MAX) {
				numberInfo.type = Numbers::TYPE_UINT64;
				numberInfo.v_64 = intBuffer;
			} else {
				numberInfo.type = Numbers::TYPE_UINT32;
				numberInfo.v_32 = intBuffer;
			}
			return true;
		}
	}

	bool Keyword(EToken& token) {
		if (!isalpha(token.sToken.str.First()))
			return false;
		
		auto type = Keywords::GetTypeFromStrHash(token.sToken.str.hash);
		if (type != Keywords::TYPE_INVALID) {
			token.type = ETokenType::KEYWORD;
			token.keywordInfo.type = type;
			return true;
		} else {
			return false;
		}
	}

	bool Operator(EToken& token) {
		using namespace Operators;

		auto& str = token.sToken.str;
		if (SParser::GetCharType(str.First()) != SParser::CHARTYPE_OP_SYMBOL)
			return false;

		token.type = ETokenType::OPERATOR;

		Type& opType = token.operatorInfo.type;
		opType = TYPE_INVALID;
		token.operatorInfo.isAssignOp = false;

		for (int i = 0; i < TYPE_COUNT; i++) {
			const char* opStr = TYPE_STRS[i];
			int len = strlen(opStr);

			if (len > str.length)
				continue;

			if (len == str.length && str == opStr) {
				opType = (Type)i;
				break;
			} else if (len == str.length - 1 && str.StartsWith(opStr)) {
				// str has 1 more char than opStr

				char last = str.Last();
				if (last == '=') {
					if (i == TYPE_GREATER_THAN) {
						opType = TYPE_GREATER_EQ_TO;		
					} else if (i == TYPE_LESS_THAN) {
						opType = TYPE_LESS_EQ_TO;
					} else if (!Operators::IsOperatorCompareOnly((Type)i)) {
						opType = (Type)i;
						token.operatorInfo.isAssignOp = true;
					} else {
						continue;
					}
					break;
				}
			}
		}

		// All operators suffixed by '=' are assignments, but with one exception, 
		//	TYPE_EQUALS which is the other way around (i.e. "==" is compare and "=" is assign)
		//	To fix this, we simply invert isAssignOp for TYPE_EQUALS
		if (opType == TYPE_EQUALS)
				token.operatorInfo.isAssignOp = !token.operatorInfo.isAssignOp;

		return true;
	}

	bool Symbol(EToken& token) {
		if (token.sToken.str.length == 1) {
			auto type = Symbols::TableLookup(token.sToken.str.First());
			if (type != Symbols::TYPE_INVALID) {
				token.type = ETokenType::UTIL_SYMBOL;
				token.symbolInfo.type = type;
				return true;
			}
		}
		return false;
	}
}

EToken EParser::Parse(SToken sToken) {
	EToken result = EToken(sToken);

	if (sToken.charType == SToken::CHARTYPE_ALNUM) {
		if (!TrySetInfoFuncs::Num(result)) // Try as a number
			if (!TrySetInfoFuncs::Keyword(result)) // Not a number? Try as keyword
				result.type = ETokenType::IDENTIFIER; // Not a keyword either? Must be an identifier

	} else if (sToken.charType == SToken::CHARTYPE_OP_SYMBOL) {
		TrySetInfoFuncs::Operator(result);
	} else if (sToken.charType == SToken::CHARTYPE_SYMBOL) {
		TrySetInfoFuncs::Symbol(result);
	} else {
		UNIMPLEMENTED;
	}

	return result;
}
