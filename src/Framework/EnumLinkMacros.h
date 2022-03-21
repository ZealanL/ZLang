// This header is dedicated to creating macros for making named arrays and such
// EXAMPLE USE CASE: We want to make an enum, each element also having a corresponding string value
// -> This macro system will allow us to do so without maintaining a seperate string array
// This is achieved via the __LINE__ macro

#pragma once

#include "RunTimeEval.h"

template <typename T>
class ELM_ArrayAccessor {
private:
	T* _coreArray;
public:

	ELM_ArrayAccessor(T* coreArray) : _coreArray(coreArray) {}

	const T& operator[](int index) const {
		return _coreArray[index];
	}
};

// Start an ELM enum
#define ELM_START(name, arrayType, arrayNameAppend, typedefName) \
typedef UINT32 typedefName; \
constexpr typedefName __ELM_BASE_##name = __LINE__ + 1; \
inline vector<arrayType> __ELM_VEC_##name; \
const auto name##_##arrayNameAppend = ELM_ArrayAccessor<arrayType>(__ELM_VEC_##name.begin()._Ptr); \
enum { name##_INVALID = -1 } 

// Append a new ELM element (creates both enum value and array)
// Note: ONE PER LINE ONLY! DO NOT SKIP ANY LINES!
#define ELM(name, elementName, val) \
constexpr UINT32 __ELM_INDEX_##name_##elementName = (__LINE__ - __ELM_BASE_##name); \
enum { name##_##elementName = __ELM_INDEX_##name_##elementName }; \
RUNTIME([] {__ELM_VEC_##name.push_back(val);}) \

// End an ELM enum
#define ELM_END(name) \
enum { name##_COUNT = __LINE__ - __ELM_BASE_##name }; \
RUNTIME([]{ ASSERT( __ELM_VEC_##name.size() == name##_COUNT ); }) // Assert that there was one ELM append per line between ELM_START and ELM_END