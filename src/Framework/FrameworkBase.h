// Base includes and defines for Framework.h

#pragma once

// STD imports
#define _HAS_STD_BYTE false
#include <stdint.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <stack>
#include <map>
#include <set>
#include <list>
#include <deque>
#include <thread>
#include <typeinfo>
#include <cassert>
#include <functional>

// Convenient using for STD
using std::vector;
using std::list;
using std::map;
using std::set;
using std::deque;
using std::string;
using std::pair;

// Standard integer definitions
typedef uint64_t	UINT64; typedef int64_t		INT64;
typedef uint32_t	UINT32; typedef int32_t		INT32;
typedef uint16_t	UINT16; typedef int16_t		INT16;
typedef uint8_t		BYTE;	typedef int8_t		INT8;

#define FINLINE __forceinline

// Debug logging
#ifdef _DEBUG
#define DLOG(s) { std::cout << std::dec << s << std::endl; }
#else
#define DLOG(s) {}
#endif

#define STRINGIFY(s) #s

#define STR(s) ([=]{std::stringstream __str__stream; __str__stream << s; return __str__stream.str();}())

#define ASSERT assert
#define S_ASSERT static_assert
#define UNIMPLEMENTED ASSERT(false)
#define SHOULD_BE_IMPOSSIBLE ASSERT(false)

#define S_ARR_LEN(a) (sizeof(a)/sizeof(*a))

#define TEMPLATE_INTEGRAL_ONLY template<typename T, class = typename std::enable_if<std::is_integral<T>::value>::type>

// Two levels of indirection needed
#define __NAMEMERGE_IDR_INNER(a, b) a##b
#define NAMEMERGE_IDR(a, b) __NAMEMERGE_IDR_INNER(a, b)

FINLINE bool __BOOL_OFF(bool& b) {
	if (b) {
		b = false;
		return true;
	} else {
		return true;
	}
}

#define DO_ONCE \
	static bool __do_once = false; \
	if (__BOOL_OFF(__do_once))