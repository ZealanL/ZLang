#pragma once
#include "Def.h"
#include "../../../CoreTypes.h"

struct RegRef {
	EReg reg;

	bool isRel; int relOffset;

	RegRef(EReg reg) {
		this->reg = reg;
		this->isRel = false;
		this->relOffset = 0;
	}

	RegRef(EReg reg, bool isRel, int relOffset) {
		this->reg = reg;
		this->isRel = isRel;
		if (isRel)
			this->relOffset = relOffset;
		else
			this->relOffset = 0;
	}

	RegRef(EReg reg, int relOffset) {
		this->reg = reg;
		this->isRel = true;
		this->relOffset = relOffset;
	}

	bool operator ==(RegRef other) {
		return (reg == other.reg) && (isRel == other.isRel) && (relOffset == other.relOffset);
	}
};

class Instruction {
public:
	vector<BYTE> bytes;

#ifdef _DEBUG
	const char* debugComment = NULL;
#endif

	Instruction(const char* debugComment) {
#ifdef _DEBUG
		this->debugComment = debugComment;
#endif
	}

	Instruction(std::initializer_list<INT64> bytesIn , const char* debugComment
	) {
		for (INT64 val : bytesIn) {
			DynamicAppend(val);
		}

#ifdef _DEBUG
		this->debugComment = debugComment;
#endif
	}

	Instruction(ZByteArray& bytes, const char* debugComment
	) {
		this->bytes = bytes.ToVec();
		
#ifdef _DEBUG
		this->debugComment = debugComment;
#endif
	}

	void DynamicAppend(INT64 code) {
		if (code > UINT16_MAX)
			Append<UINT32>(code);
		else if (code > UINT8_MAX)
			Append<UINT16>(code);
		else
			Append<BYTE>(code);
	}

	template <typename T>
	void Append(T val) {
		BYTE* asByteArray = (BYTE*)&val;
		for (int i = 0; i < sizeof(T); i++) {
			bytes.push_back(asByteArray[i]);
		}
	}

	void AppendBytes(ZByteArray bytes) {
		for (BYTE b : bytes)
			this->bytes.push_back(b);
	}

	int Size() {
		return bytes.size();
	}

	bool IsValid() {
		return Size() > 0;
	}

	string ToString(bool brackets = false) {
		std::stringstream stream;

		if (!IsValid()) {
			return brackets ? "[]" : "";
		}

		if (brackets)
			stream << "[ ";

		stream << std::hex;
		for (BYTE b : bytes) {
			stream << std::setw(2) << std::setfill('0') << (int)b << ' ';
		}

		if (brackets)
			stream << "]";

#ifdef _DEBUG
		if (brackets && debugComment)
			stream << " // " << debugComment;
#endif

		return stream.str();
	}
};

#define INSTRUCTION_INVALID Instruction("INSTRUCTION_INVALID")