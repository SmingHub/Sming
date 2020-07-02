#pragma once

#include <SmingCore.h>
#include <Data/Stream/ReadWriteStream.h>
#include "HostedCommon.h"

class HostedClient
{
public:

	HostedClient(ReadWriteStream* stream): stream(stream)
    {
	}

	void setStream(ReadWriteStream* stream)
	{
		if(this->stream != nullptr) {
			debug_w("Discarding current stream!");
		}

		this->stream = stream;
	}

	bool send(HostedCommand* message, HostedCommandDelegate callback = nullptr)
	{
		if(++messageId == 0) {
			messageId = 1; // messages with id 0 will be discarded...
		}
		message->id = messageId;

		pb_ostream_t ouput = newOutputStream();
		bool success = pb_encode_ex(&ouput, HostedCommand_fields, message, PB_ENCODE_DELIMITED);
		if (!success) {
			debug_e("Encoding failed: %s\n", PB_GET_ERROR(&ouput));
			return false;
		}

		if(callback != nullptr) {
			responseCallbacks[message->id] = callback;
		}

		return true;
	}

	/**
	 * @brief This method handles incoming data
	 */
	bool onData(const char* at, size_t length)
	{
		return true;
	}
private:
	pb_ostream_t newOutputStream()
	{
		pb_ostream_t outputStream;
		outputStream.callback = [](pb_ostream_t *stream, const pb_byte_t *buf, size_t count) -> bool {
			ReadWriteStream* destination = (ReadWriteStream* )stream->state;
			size_t written = destination->write((const uint8_t *)buf, count);

			return (written == count);
		};
		outputStream.state = (void*)this->stream;
		outputStream.max_size = SIZE_MAX;
		outputStream.bytes_written = 0;
		outputStream.errmsg = nullptr;

		return outputStream;
	}
private:
	HashMap<uint32_t,HostedCommandDelegate> responseCallbacks;
	uint32_t messageId = 0;
	ReadWriteStream* stream = nullptr;
};
