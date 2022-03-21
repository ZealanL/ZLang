#pragma once
#include "../CoreTypes.h"

// ZIL = ZLang Intermediate Language

#pragma region ZIL Variables
// >0 = local
// <0 = global
//  0 = Invalid
#define VARID_INVALID 0
struct VarId {
	int val;
	operator int() { return val; }

	VarId() { this->val = 0; }
	VarId(int val) : val(val) {}

	bool IsValid() {
		return val != VARID_INVALID;
	}

	string ToString() {
		if (!val) {
			return "ERRVAL";
		} else {
			return (val > 0 ? 'L' : 'G') + STR(abs(val));
		}
	}
	operator string() {
		return ToString();
	}

	friend std::ostream& operator <<(std::ostream& stream, VarId& var) {
		stream << var.ToString();
		return stream;
	}
};

struct ZILVar {
	VarId id;
	ZByteArray bytes;

	bool expired = false; // This variable is no longer used and can be repurposed

	ZILVar(VarId id, ZByteArray bytes) : bytes(bytes), id(id) {}

	string ToString() {
		return STR("VAR " << id << "[" << bytes.Size() << "] = " << bytes.ToString());
	}
};

// Reference to a ZIL var, with an optional relative read and optional relative read offset
struct VarRef {
	VarId id;

	// Actual size of referenced data
	// If not relative, the variable size
	// Otherwise, the size of data at the relative offset
	int actualSize;

	bool isRel;
	int relOffset;

	VarRef() {
		this->id = VARID_INVALID;
		this->actualSize = 0;

		this->isRel = false;
		this->relOffset = 0;
	}

	VarRef(VarId id, int actualSize, bool relative = false, int relativeOffset = 0) {
		this->id = id;
		this->actualSize = actualSize;
		this->isRel = relative;
		this->relOffset = relativeOffset;
	}

	bool operator ==(VarRef other) {
		return id == other.id && actualSize == other.actualSize && isRel == other.isRel && relOffset == other.relOffset;
	}

	bool IsValid() {
		return id.IsValid() && actualSize > 0;
	}

	string ToString() {
		std::stringstream stream;

		if (isRel)
			stream << '[';

		stream << id;

		if (isRel && relOffset) {
			if (relOffset > 0)
				stream << '+';
			stream << relOffset;
		}

		if (isRel)
			stream << ']';

		return stream.str();
	}
};

// Either a ZILVarRef, or raw byte value
struct VarRefOrVal {
	bool isVar = false;
	union {
		VarRef varRef;
		ZByteArray byteData;
	};

	VarRefOrVal() {}
	VarRefOrVal(VarRef varRef) {
		this->isVar = true;
		this->varRef = varRef;
	}

	VarRefOrVal(ZByteArray byteData) {
		this->isVar = false;
		this->byteData = byteData;
	}

	string ToString() {
		if (!isVar) {
			return byteData.ToString();
		} else {
			return varRef.ToString();
		}
	}
	
	int GetSize() {
		if (isVar) {
			return varRef.actualSize;
		} else {
			return byteData.Size();
		}
	}

	bool IsValid() {
		if (isVar) {
			return varRef.IsValid();
		} else {
			return !byteData.IsEmpty();
		}
	}

	~VarRefOrVal() {}
};
#pragma endregion

#pragma region ZIL Instructions
// Header for all instructions
typedef UINT16 ZILIHeader;

// Base struct for all instructions
class ZILIBase {
public:
	virtual string ToString() = 0;
	operator string() { return ToString(); }

	virtual bool IsValid() = 0;
	virtual int GetId() = 0;

	ZILIBase() {}
};

// Begin-define an instruction
#define ZILI(name, val) \
enum { EZIL_##name = val }; \
class ZILI_##name : public ZILIBase

#define ZILI_START(name) \
public: \
ZILI_##name(){} \
int GetId() { return EZIL_##name; }

#pragma endregion

#pragma region ZIL Execution Scopes/Area
#define MAX_LOCAL_VARS INT_MAX // 2^31 - 1
#define MAX_GLOBAL_VARS MAX_LOCAL_VARS
struct ZILExecutionScope {
	vector<ZILVar> localVars;

	// TODO: Check for resuable expired local variables with same size
	ZILVar* MakeNewVar(ZByteArray bytes) {
		localVars.push_back( ZILVar(VarId(localVars.size() + 1), bytes) );
		return &localVars.back();
	}

	ZILVar* GetLocalVar(VarId id) {
		ASSERT(id > 0);
		ASSERT(id <= localVars.size());
		return &localVars[id - 1];
	}

	ZILVar* GetGlobalVar(VarId id) {
		// TODO: Implement
		return NULL;
	}

	string LocalsToString() {
		std::stringstream outStream;

		for (auto& var : localVars) {
			outStream << var.ToString() << std::endl;
		}

		return outStream.str();
	}
};

struct ZILExecutionArea {
	ZILExecutionScope scope;
	vector<ZILIBase*> instructions;

	bool IsEmpty() {
		return scope.localVars.empty() && instructions.empty();
	}

	template <typename T, typename = std::enable_if<std::is_base_of<ZILIBase, T>::value>>
	void AddInstruction(T inst) {
		T* pInst = new T(inst);
		instructions.push_back(pInst);
	}

	string InstructionsToString() {
		std::stringstream outStream;

		outStream << "{\n";
		outStream << scope.LocalsToString();
		outStream << std::endl;
		for (auto inst : instructions) {
			outStream << "  > " << inst->ToString() << std::endl;
		}
		outStream << "}\n";

		return outStream.str();
	}

	~ZILExecutionArea() {
		for (ZILIBase* inst : instructions) {
			delete inst;
		}
	}
};
#pragma endregion

#pragma region ZIL Function
struct ZILFunction {
	string name;
	ZILExecutionArea area;
};
#pragma endregion