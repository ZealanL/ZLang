// Compile time string hashes

#pragma once
#include "FrameworkBase.h"

typedef UINT64 HASH;

namespace CompTimeHash {
	constexpr int TABLE_SIZE = 4;

	// Large fibonnaci prime
	constexpr HASH START_HASH = 99194853094755497;

	constexpr HASH TABLE[TABLE_SIZE] {
		0x77073096ee0e612c, 0x990951ba076dc419,
		0x706af48e963a535, 0x9e6495a30edb8832,
	};

	constexpr HASH DoHash(const char* str, HASH const size) {
		HASH out = START_HASH;

		for (int i = 0; i < size; i++) {
			HASH curMix = out ^ TABLE[i % TABLE_SIZE];
			HASH curChar = str[i];
			out ^= (curMix * curChar) + curMix + curChar;
		}

		return out;
	}
}