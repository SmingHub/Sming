// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// Sming FlashString adapter mikee47 April 2019 <mike@sillyhouse.net>

#pragma once

namespace ARDUINOJSON_NAMESPACE
{
class FlashStringRefAdapter
{
public:
	FlashStringRefAdapter(const FlashString& str) : _str(str)
	{
	}

	bool equals(const char* expected) const
	{
		return _str.isEqual(expected);
	}

	bool isNull() const
	{
		return false;
	}

	char* save(MemoryPool* pool) const
	{
		size_t n = _str.size();
		char* dup = pool->allocFrozenString(n);
		if(dup)
			memcpy_P(dup, _str.data(), n);
		return dup;
	}

	const char* data() const
	{
		return nullptr;
	}

	size_t size() const
	{
		return _str.length();
	}

	bool isStatic() const
	{
		return false;
	}

private:
	const FlashString& _str;
};

inline FlashStringRefAdapter adaptString(const FlashString& str)
{
	return FlashStringRefAdapter(str);
}

template <> struct IsString<FlashString> : true_type {
};

} // namespace ARDUINOJSON_NAMESPACE
