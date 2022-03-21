#pragma once

#include "../Lang.h"



struct LangContext {
	enum {
		TYPE_INVALID = -1,

		TYPE_DEF_TYPE,
		TYPE_DEF_FUNCTION,

		TYPE_COUNT,
	};

	int type = TYPE_INVALID;

	struct TypeDefInfo {
		ZLT_Type* type;
	};

	struct FuncDefInfo {
		ZLT_Function* func;
	};

	union {
		TypeDefInfo typeDefInfo;
		FuncDefInfo funcDefInfo;
	};
};