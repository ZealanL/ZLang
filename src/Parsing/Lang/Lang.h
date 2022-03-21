#pragma once
#include "../../CoreTypes.h"



//////////////////////////

struct ZLT_Variable {
	struct ZLT_Type* type;
	ZLT_Str name;

	bool isReference;
};

struct ZLT_SearchVars : ZSHArray<ZLT_Variable> {

	static HASH GetVarHash(const ZLT_Variable& var) {
		return var.name.hash;
	}

	ZLT_SearchVars() : ZSHArray<ZLT_Variable>(&GetVarHash) {}
};

struct ZLT_Type {
	ZLT_Str name;
	bool isNativeType = false;

	ZLT_SearchVars vars;
};

////////////////////////////////////

struct ZLT_Expression {
	ZLT_Expression* parent; // Null if no parent

	list<ZLT_Expression> children;
};

// A line of executed code
struct ZLT_CodeLine {
	enum {
		TYPE_INVALID = -1,



		TYPE_COUNT
	};
};

// An area where executed code is, with local variables and ZLT_CodeLines
struct ZLT_CodeScope {
	ZLT_CodeScope* parent; // Null if no parent
	ZLT_SearchVars localVars;
	list<ZLT_CodeLine> lines;
};

struct ZLT_Function {
	ZLT_Str name;

	ZLT_SearchVars args;
	ZLT_Type* returnType; // Null if no return

	ZLT_CodeScope mainScope;
};