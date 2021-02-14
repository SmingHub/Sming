#include "HostedUtils.h"
#include <Data/Stream/MemoryDataStream.h>

// See: https://iam777.tistory.com/538

bool pbEncodeData(pb_ostream_t* stream, const pb_field_t* field, void* const* arg)
{
	auto data = static_cast<PbData*>(*arg);
	if(data == nullptr) {
		return false;
	}

	if(!pb_encode_tag_for_field(stream, field)) {
		return false;
	}

	return pb_encode_string(stream, (uint8_t*)data->value, data->length);
}

bool pbDecodeData(pb_istream_t* stream, const pb_field_t* field, void** arg)
{
	uint8_t buffer[1024]{};

	/* We could read block-by-block to avoid the large buffer... */
	if(stream->bytes_left >= sizeof(buffer)) {
		return false;
	}

	size_t available = stream->bytes_left;
	if(!pb_read(stream, buffer, stream->bytes_left)) {
		return false;
	}

	auto data = static_cast<MemoryDataStream*>(*arg);
	if(data == nullptr) {
		data = new MemoryDataStream();
		*arg = data;
	}
	data->write(buffer, available);
	return true;
}
