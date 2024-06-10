// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// Sming FlashString adapter mikee47 April 2019 <mike@sillyhouse.net>

#pragma once

#include <FlashString/String.hpp>
#include <ArduinoJson/Strings/Adapters/FlashString.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

template <> struct StringAdapter<FSTR::String> {
	typedef FlashString AdaptedString;

	static AdaptedString adapt(const FSTR::String& str)
	{
		return FlashString(str.data(), str.length());
	}
};

inline CompareResult compare(JsonVariantConst lhs, const FSTR::String& rhs)
{
	return compare(lhs, String(rhs));
}

ARDUINOJSON_END_PRIVATE_NAMESPACE
