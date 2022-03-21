#pragma once
#include "../ZIL.h"

// ZIL Instruction Defintions
#pragma pack(push, 1) // CRUCIAL: Don't allow padding of ZILI structs

// 1, 2
#pragma region Variable Setting


ZILI(Set, 1) {
	ZILI_START(Set);

	VarRef destVar, srcVar;

	string ToString() {
		return STR("set " << destVar.ToString() << " = " + srcVar.ToString());
	}

	bool IsValid() {
		return destVar.IsValid() && srcVar.IsValid();
	}
};

ZILI(SetVal, 2) {
	ZILI_START(SetVal);

	VarRef var;
	ZByteArray valBytes;

	string ToString() {
		return STR("setval " << var.ToString() << " = 0x" << valBytes.ToString());
	}

	bool IsValid() {
		return var.IsValid() && !valBytes.IsEmpty();
	}
};
#pragma endregion

// 20, 30
#pragma region Math
enum class ZILI_MATH_OP : BYTE {
	INVALID,
	ADD, SUB, MULT, DIV,
	OR, AND, XOR,

	NEGATE, ABS,

	SHIFT_LEFT,
	SHIFT_RIGHT,

	COUNT
};

inline string ZILI_GetOpSymbol(ZILI_MATH_OP opNum) {
	switch (opNum) {
	case ZILI_MATH_OP::ADD: return "+";
	case ZILI_MATH_OP::SUB: return "=";
	case ZILI_MATH_OP::MULT: return "*";
	case ZILI_MATH_OP::DIV: return "/";

	default: return "[UNKNOWN OPERATION]";
	}
}

enum class ZILI_MATH_FLT_PERCISION : BYTE {
	INVALID,
	SINGLE, DOUBLE
};

inline string ZILI_GetFltPercisionSymbol(ZILI_MATH_FLT_PERCISION percNum) {
	switch (percNum) {
	case ZILI_MATH_FLT_PERCISION::SINGLE: return "S";
	case ZILI_MATH_FLT_PERCISION::DOUBLE: return "D";

	default: return "[UNKNOWN PERCISION]";
	}
}

inline int ZILI_GetFltPercisionSize(ZILI_MATH_FLT_PERCISION percNum) {
	switch (percNum) {
	case ZILI_MATH_FLT_PERCISION::SINGLE: return 4;
	case ZILI_MATH_FLT_PERCISION::DOUBLE: return 8;

	default: return -1;
	}
}

ZILI(IntMath, 20) {
	ZILI_START(IntMath)

	VarRef inputVar;
	VarRef outputVar; // Where we are outputting the result to (can be same as inputVar)
	ZILI_MATH_OP operation;

	// NOTE: Unary operations have no operative value
	VarRefOrVal opVal;

	bool isSigned;

	string ToString() {
		return STR("intmath[" << (isSigned ? 'S' : 'U') << "] " << outputVar.ToString() << " = " << inputVar.ToString() << " " << ZILI_GetOpSymbol(operation) << " " << opVal.ToString());
	}

	bool IsValid() {
		return inputVar.IsValid() && opVal.IsValid() && outputVar.IsValid()
			&& operation != ZILI_MATH_OP::INVALID;
	}
};

ZILI(FltMath, 30) {
	ZILI_START(FltMath);

	VarId inputVar;

	VarId outputVar; // Where we are outputting the result to (can be same as inputVar)

	ZILI_MATH_FLT_PERCISION percision;
	ZILI_MATH_OP operation;

	// NOTE: Unary operations have no operative value
	VarRefOrVal opVal;

	string ToString() {
		return STR("fltmath[" << ZILI_GetFltPercisionSymbol(percision) << "] " << outputVar.ToString() + " = "
			<< inputVar.ToString() << " " + ZILI_GetOpSymbol(operation) << " " << opVal.ToString());
	}

	bool IsValid() {

		return inputVar.IsValid() && opVal.IsValid() && outputVar.IsValid()
			&& percision != ZILI_MATH_FLT_PERCISION::INVALID && operation != ZILI_MATH_OP::INVALID;
	}
};

#pragma endregion

// 50, 51, 60
#pragma region Control Flow

ZILI(JumpToInst, 50) {
	ZILI_START(JumpToInst);

	int targetInstructionIndex = -1;

	string ToString() {
		return STR("jmpi " << targetInstructionIndex);
	}

	bool IsValid() {
		return targetInstructionIndex != -1;
	}
};

ZILI(JumpToVal, 51) {
	ZILI_START(JumpToVal);

	VarId addressVal;

	string ToString() {
		return STR("jmpv " << addressVal);
	}
	bool IsValid() {
		return addressVal.IsValid();
	}
};


ZILI(CallFunc, 60) {
	ZILI_START(CallFunc);

	int funcIndex = -1;
	vector<VarRefOrVal> args;

	string ArgsToString() {
		if (args.empty()) {
			return "()";
		} else {

			std::stringstream stream;

			stream << "( ";
			for (int i = 0; i < args.size(); i++) {
				if (i > 0)
					stream << ", ";

				stream << args[i].ToString();
			}

			stream << " )";

			return stream.str();
		}
	}

	string ToString() {
		return STR("call " << funcIndex << " " << ArgsToString());
	}

	bool IsValid() {
		return funcIndex != -1;
	}
};

#pragma endregion

#pragma pack(pop) // CRUCIAL: Only do this here