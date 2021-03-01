/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Stream.cpp
 *
 ****/

#include "include/Protobuf/Stream.h"

// See: https://iam777.tistory.com/538

namespace Protobuf
{
bool InputStream::decode(const pb_msgdesc_t* fields, void* dest_struct)
{
	int avail = stream.available();
	if(avail <= 0) {
		return false;
	}
	pb_istream_t is{};
	is.callback = [](pb_istream_t* stream, pb_byte_t* buf, size_t count) -> bool {
		auto self = static_cast<InputStream*>(stream->state);
		assert(self != nullptr);
		size_t read = self->stream.readBytes(reinterpret_cast<char*>(buf), count);
		return true;
	};
	is.state = this;
	is.bytes_left = size_t(avail);
	is.errmsg = nullptr;
	return pb_decode(&is, fields, dest_struct);
}

size_t OutputStream::encode(const pb_msgdesc_t* fields, const void* src_struct)
{
	pb_ostream_t os{};
	os.state = const_cast<OutputStream*>(this);
	os.callback = buf_write;
	os.max_size = SIZE_MAX;

	return pb_encode(&os, fields, src_struct) ? os.bytes_written : 0;
}

} // namespace Protobuf
