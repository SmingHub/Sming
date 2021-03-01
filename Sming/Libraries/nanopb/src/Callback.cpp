/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Callback.cpp
 *
 ****/

#include "include/Protobuf/Callback.h"
#include <pb_encode.h>
#include <pb_decode.h>

// See: https://iam777.tistory.com/538

namespace Protobuf
{
bool OutputCallback::encode(pb_ostream_t* stream, const pb_field_t* field)
{
	if(!pb_encode_tag_for_field(stream, field)) {
		return false;
	}

	return pb_encode_string(stream, data, length);
}

bool InputCallback::decode(pb_istream_t* stream, const pb_field_t* field)
{
	size_t available = stream->bytes_left;
	auto new_buf = realloc(data, length + available);
	if(new_buf == nullptr) {
		return false;
	}
	data = static_cast<uint8_t*>(new_buf);
	auto old_length{length};
	length += available;

	return pb_read(stream, &data[old_length], available);
}

} // namespace Protobuf
