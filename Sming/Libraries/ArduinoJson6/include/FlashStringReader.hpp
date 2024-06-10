// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2019
// MIT License
//
// Sming FlashString reader mikee47 Nov 2019 <mike@sillyhouse.net>

#pragma once

#include <FlashString/String.hpp>
#include <ArduinoJson/Deserialization/Readers/FlashReader.hpp>

ARDUINOJSON_BEGIN_PRIVATE_NAMESPACE

template <> struct Reader<const FSTR::String, void> : public BoundedReader<const __FlashStringHelper*, void> {
	explicit Reader(const FSTR::String& str) : BoundedReader(str.data(), str.length())
	{
	}
};

ARDUINOJSON_END_PRIVATE_NAMESPACE
