#pragma once

#include <SmingCore.h>
#include <pb_encode.h>
#include <pb_decode.h>
#include "hosted.pb.h"

struct PbData {
	uint8_t* value;
	size_t length;
};

bool pbEncodeData(pb_ostream_t* stream, const pb_field_t* field, void* const* arg);
bool pbDecodeData(pb_istream_t* stream, const pb_field_t* field, void** arg);
