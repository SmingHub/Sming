#pragma once

#include <SmingCore.h>
#include <Data/Stream/MemoryDataStream.h>
#include <pb_encode.h>
#include <pb_decode.h>

class PbData
{
public:
	uint8_t* value = nullptr;
	size_t length = 0;

	PbData(const String& text)
	{
		PbData((uint8_t *)text.c_str(), text.length());
	}

	PbData(uint8_t*  data, size_t length)
	{
		value = data;
		this->length = length;
	}
};

bool pbEncodeData(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool pbDecodeData(pb_istream_t *stream, const pb_field_t *field, void **arg);
