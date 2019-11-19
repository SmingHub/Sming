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
	FlashStringRefAdapter(const FlashString& str) : str(str)
	{
	}

	bool equals(const char* expected) const
	{
		return str.equals(expected);
	}

	bool isNull() const
	{
		return str.isNull();
	}

	char* save(MemoryPool* pool) const
	{
		size_t n = str.size();
		char* dup = pool->allocFrozenString(n);
		if(dup) {
			str.read(0, dup, n);
		}
		return dup;
	}

	const char* data() const
	{
		// Cannot access directly using a char*
		return nullptr;
	}

	size_t size() const
	{
		return str.length();
	}

	bool isStatic() const
	{
		// Whilst  our value won't change, it cannot be accessed using a regular char*
		return false;
	}

private:
	const FlashString& str;
};

inline FlashStringRefAdapter adaptString(const FlashString& str)
{
	return FlashStringRefAdapter(str);
}

template <> struct IsString<FlashString> : true_type {
};

} // namespace ARDUINOJSON_NAMESPACE
