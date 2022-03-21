#pragma once

#include "Framework/FrameworkBase.h"

// FrameworkUtil headers
#include "Framework/CompTimeHash.h"
#include "Framework/EnumLinkMacros.h"
#include "Framework/RunTimeEval.h"

// Disabled warnings
#pragma warning( disable : 4507)

// Treats warning "not all control paths return a value" as an error
#pragma warning(error : 4175)

//////////////////////////////////////////

// Compile-time hash
#define CHASH(s) CompTimeHash::DoHash(s, sizeof(s)-1)

// Framework functions
#define FWFUNC inline
namespace FW {

	// Hash a string with a given size
	// String length is determined automatically if size argument is -11
	FWFUNC HASH Hash(const char* str, UINT32 size = -1) {
		if (size == -1)
			size = strlen(str);

		return CompTimeHash::DoHash(str, size);
	};

	TEMPLATE_INTEGRAL_ONLY
	FWFUNC bool IsPowerOf2(T n) {
		return n && (n & (n - 1)) == 0;
	}

	// NOTE: Assumes the input is a power of 2
	FWFUNC UINT32 FastLog2_U32(UINT32 n) {
		return (sizeof(n) * 8) - __lzcnt(n);
	}

	TEMPLATE_INTEGRAL_ONLY
		FWFUNC T RoundUp(T n, T alignMult) {
		T remainder = n % alignMult;
		if (remainder == 0) {
			return n;
		} else {
			return n - remainder + alignMult;
		}
	}

	// Framework string functions
	namespace S {
		FWFUNC string ToLower(const string& str) {
			string out = str;
			for (char& c : out) c = tolower(c);
			return out;
		}

		FWFUNC string ToUpper(const string& str) {
			string out = str;
			for (char& c : out) c = toupper(c);
			return out;
		}

		FWFUNC bool StartsWith(const string& str, const string& start) {
			return str.rfind(start, 0) == 0;
		}

		FWFUNC bool EndsWith(const string& str, const string& end) {
			if (end.size() > str.size()) return false;
			return equal(end.rbegin(), end.rend(), str.rbegin());
		}

		FWFUNC bool InTextMatch(const string& target, const string& text, int startIndex) {
			for (int i = 0;; i++) {
				if (target[i] == 0)
					return true;

				if (text[i + startIndex] == 0)
					return false;

				if (target[i] != text[i + startIndex])
					return false;
			}
			return true;
		}
	}
}