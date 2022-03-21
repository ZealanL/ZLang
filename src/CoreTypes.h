// The core types of ZLang

#pragma once
#include "Framework.h"

// Non-dynamic string with hash
struct ZLT_Str {
	void _Allocate() {
		strData = (char*)malloc(length+1);
	}

	void _ComputeHash() {
		this->hash = FW::Hash(strData, this->length);
	}

	char* strData;
	int length;
	HASH hash;

	ZLT_Str() {
		strData = NULL;
		length = 0;
		hash = 0;
	}

	ZLT_Str(const ZLT_Str& other) noexcept {
		this->length = other.length;
		this->hash = other.hash;
		_Allocate();

		if (other.strData) {
			memcpy(strData, other.strData, length + 1);
		} else {
			strData = NULL;
		}
	}

	ZLT_Str(string str) {
		this->length = str.size();
		_Allocate();
		memcpy(this->strData, str.c_str(), this->length + 1);
		_ComputeHash();
	}

	ZLT_Str(const char* str) {
		this->length = strlen(str);
		_Allocate();
		memcpy(this->strData, str, this->length + 1);
		_ComputeHash();
	}

	char& operator[](int index) {
		return strData[index];
	}

	char& First() {
		ASSERT(length > 0);
		return strData[0];
	}

	char& Last() {
		ASSERT(length > 0);
		return strData[length-1];
	}

	ZLT_Str ToCase(bool upper) {
		ZLT_Str copy = *this;
		for (char& c : copy)
			c = upper ? toupper(c) : tolower(c);
		return copy;
	}

	ZLT_Str ToLower() { return ToCase(false); }
	ZLT_Str ToUpper() { return ToCase(true); }

	// An array access, returns NULL char if out of bounds
	char TryGet(int index) {
		return (index >= 0 && index < length) ? strData[index] : NULL;
	}

	bool HasChar(char charToFind) {
		for (char c : *this)
			if (c == charToFind)
				return  true;

		return false;
	}

	char* Find(const char* subStr) {
		return strstr(strData, subStr);
	}

	bool StartsWith(const char* subStr) {
		for (int i = 0; i < length + 1; i++) {
			
			if (!subStr[i])
				return true;

			// This will check to make sure terminators match as well - assuming subStr's length is >= our length
			if (subStr[i] != strData[i])
				return false;
		}

		return true;
	}

	bool EndsWith(const char* subStr) {
		int subStrLen = strlen(subStr);

		if (subStrLen == length)
			return this->operator==(subStr);

		if (subStrLen > length)
			return false;

		int shift = length - subStrLen;
		for (int i = 0; i < subStrLen; i++) {
			if (strData[i + shift] != subStr[i])
				return false;
		}

		return true;
	}

	bool operator ==(const ZLT_Str& other) {
		return this->hash == other.hash;
	}

	bool operator ==(const char* other) {
		return !strcmp(strData, other);
	}

	operator const string() const {
		return strData ? strData : "";
	}

	~ZLT_Str() {
		if (strData)
			free(strData);
	}

	// For C++ iterator
	char* begin() { return strData; }
	char* end() { return strData + length; }

	// For std::ostream concat
	friend std::ostream& operator <<(std::ostream& stream, const ZLT_Str& zltStr) {
		if (zltStr.strData)
			stream << zltStr.strData;
		return stream;
	}
};

struct ZByteArray {
	void _Allocate() {
		_vals = (BYTE*)malloc(_size);
	}

	BYTE* _vals;
	UINT32 _size;

	ZByteArray() {
		_vals = 0;
		_size = 0;
	}

	ZByteArray(const ZByteArray& other) {
		_size = other._size;
		if (_size) {
			_Allocate();
			memcpy(_vals, other._vals, _size);
		}
	}

	ZByteArray(UINT32 size) {
		_size = size;
		if (_size > 0) {
			_Allocate();
			memset(_vals, 0, size);
		}
	}

	ZByteArray(BYTE* beginPtr, UINT32 size) {
		_size = size;
		if (_size > 0) {
			_Allocate();
			memcpy(_vals, beginPtr, size);
		}
	}

	template <typename T>
	ZByteArray(std::initializer_list<T> initBytes) {
		_size = initBytes.size();
		_Allocate();

		if (sizeof(T) == 1) {
			memcpy(_vals, initBytes.begin(), _size);
		} else {
			for (int i = 0; i < initBytes.size(); i++)
				_vals[i] = *(initBytes.begin() + i);
		}
	}

	~ZByteArray() {
		if (_vals)
			free(_vals);
	}

	string ToString(bool brackets = true) {
		if (Size() == 0)
			return brackets ? "[]" : "";

		std::stringstream out;

		if (brackets)
			out << "[ ";

		out << std::hex;

		for (BYTE val : *this) {
			out << std::setw(2) << std::setfill('0') << (int)val << ' ';
		}

		if (brackets)
			out << "]";

		return out.str();
	}

	UINT32 Size() {
		return _size;
	}

	bool IsEmpty() {
		return _size == 0;
	}

	BYTE* Base() {
		return _vals;
	}

	BYTE& First() {
		return _vals[0];
	}

	BYTE& Last() {
		return _vals[_size - 1];
	}

	vector<BYTE> ToVec() {
		return vector<BYTE>(Base(), Base() + _size);
	}

	BYTE& operator[](UINT32 index) {
		ASSERT(index < _size);
		return _vals[index];
	}

	// Read a type of a given size from these bytes, starting at a specific index
	// If the read goes out of bounds, the output will be padded with a pad byte
	template <typename T>
	T Read(UINT32 index = 0, BYTE pad = 0) {
		if (index + sizeof(T) >= _size) {
			// This will go out of bounds, pad accordingly
			BYTE data[sizeof(T)];
			for (int i = 0; i < sizeof(T); i++) {
				if (index + i >= _size) {
					data[i] = 0;
				} else {
					data[i] = (*this)[index + i];
				}
			}

			return *(T*)data;

		} else {
			return *(T*)(_vals + index);
		}
	}

	// For C++ iterator
	BYTE* begin() { return &_vals[0]; }
	const BYTE* begin() const { return &_vals[0]; }
	BYTE* end() { return &_vals[_size]; }
	const BYTE* end() const { return &_vals[_size]; }
};

struct ZByteStream {
	vector<BYTE> _bytes;

	void Clear() {
		_bytes.clear();
	}

	int Size() {
		return _bytes.size();
	}

	bool IsEmpty() {
		return Size() == 0;
	}

	BYTE& operator[](int index) {
		return _bytes[index];
	}

	void WriteByte(BYTE val) {
		_bytes.push_back(val);
	}

	void WriteBytes(ZByteArray newBytes) {
		_bytes.reserve(Size() + newBytes._size);

		for (BYTE b : newBytes)
			_bytes.push_back(b);
	}

	BYTE* GetArrayPtr() {
		if (IsEmpty())
			return NULL;

		return &_bytes[0];
	}

	string ToString() {
		std::stringstream out;
		out << "[ " << std::hex;

		for (int i = 0; i < Size(); i++) {
			out << std::setw(2) << std::setfill('0') << (int)_bytes[i];
		}

		out << " ]";

		return out.str();
	}
};

// Searchable hashes array - can store any datatype that has a hash property
// NOTE: Guarentees the elements' pointers don't ever change after being added
template <typename T>
struct ZSHArray {
	std::function<HASH(const T&)> _fnGetHash;
	map<HASH, T> _internalMap;

	T* Find(HASH hash) {
		auto found = _internalMap.find(hash);
		if (found != _internalMap.end()) {
			return &found->second;
		} else {
			return NULL;
		}
	}

	bool Add(T val) {
		auto insertResult = _internalMap.insert({ _fnGetHash(val), val });

		assert(insertResult.second);
		return insertResult.second;
	}

	ZSHArray(std::function<HASH(const T&)> fnGetHash) {
		this->_fnGetHash = fnGetHash;
	}

	// For C++ iterator
	auto begin() { return _internalMap.begin(); }
	auto end() { return _internalMap.end(); }

	int Size() {
		return _internalMap.size();
	}
};