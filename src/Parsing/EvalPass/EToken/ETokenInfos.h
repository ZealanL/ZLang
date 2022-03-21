#pragma once
#include "../../../CoreTypes.h"

namespace Symbols {
#define MAKE_OC(name, o, c) \
	ELM(TYPE, name##_O, o); \
	ELM(TYPE, name##_C, c)

	ELM_START(TYPE, char, CHARS, Type);
	MAKE_OC(PAREN, '(', ')');
	MAKE_OC(BRACKET, '[', ']');
	ELM_END(TYPE);

	FINLINE Type TableLookup(char c) {
		static Type* charTypeLookupTable = nullptr;
		DO_ONCE {
			charTypeLookupTable = new Type[0xFF];
			for (int i = 0; i < 0xFF; i++) {
				Type type = TYPE_INVALID;
				for (int j = 0; j < TYPE_COUNT; j++) {
					if (TYPE_CHARS[j] == i) {
						type = i;
						break;
					}
				}
				charTypeLookupTable[i] = type;
			}
		}

		return charTypeLookupTable[c];
	}
	
	struct SymbolInfo {
		Type type = TYPE_INVALID;
	};
#undef MAKE_OC
}

namespace Keywords {

#define MAKE(name) ELM(TYPE, name, FW::Hash(FW::S::ToLower(#name).c_str()))

	ELM_START(TYPE, HASH, STRHASHES_ARRAY, Type);
	MAKE(TYPE);
	MAKE(FUNC);
	MAKE(AS);
	MAKE(IF);
	MAKE(ELSE);
	MAKE(ELIF);
	MAKE(SKIP);
	MAKE(BREAK);
	ELM_END(TYPE);

	FINLINE Type GetTypeFromStrHash(HASH hash) {
		static map<HASH, Type> typeMap;
		DO_ONCE {
			for (int i = 0; i < TYPE_COUNT; i++)
				typeMap.insert({ i, TYPE_STRHASHES_ARRAY[i] });
		}

		auto found = typeMap.find(hash);
		return (found == typeMap.end()) ? found->second : TYPE_INVALID;
	}

	struct KeywordInfo {
		Type type = TYPE_INVALID;
	};

#undef MAKE
}

namespace Numbers {
	enum Type {
		TYPE_INVALID = -1,

		// Integers
		TYPE_INT8,
		TYPE_BYTE,

		TYPE_INT16,
		TYPE_UINT16,

		TYPE_INT32,
		TYPE_UINT32,

		TYPE_INT64,
		TYPE_UINT64,

		// Floating point
		TYPE_FLOAT32,
		TYPE_FLOAT64,
	};

	struct NumberInfo {
		Type type = TYPE_INVALID;
		union {
			BYTE	v_8;
			UINT16	v_16;
			UINT32	v_32;
			UINT64	v_64;

			float	v_f32;
			double	v_f64;
		};
	};
}

namespace Operators {

	ELM_START(TYPE, const char*, STRS, Type);
	ELM(TYPE, EQUALS,			"=");
	ELM(TYPE, ADD,				"+");
	ELM(TYPE, SUBTRACT_NEGATE,	"-");
	ELM(TYPE, MULTIPLY,			"*");
	ELM(TYPE, DIVIDE,			"/");
	ELM(TYPE, MODULO,			"%");
	ELM(TYPE, SHIFT_L,			"<<");
	ELM(TYPE, SHIFT_R,			">>");
	ELM(TYPE, NOT,				"!");
	ELM(TYPE, LESS_THAN,		"<");
	ELM(TYPE, LESS_EQ_TO,		"<=");
	ELM(TYPE, GREATER_THAN,		">");
	ELM(TYPE, GREATER_EQ_TO,	">=");
	ELM_END(TYPE);
	
	inline bool IsOperatorCompareOnly(Type type) {
		return type > TYPE_SHIFT_R;
	}

	struct OperatorInfo {
		Type type = TYPE_INVALID;
		bool isAssignOp = false;
	};
}