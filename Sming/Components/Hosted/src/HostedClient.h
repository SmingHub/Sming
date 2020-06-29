#pragma once

#include <SmingCore.h>
#include <Data/Stream/ReadWriteStream.h>

#include <pb_encode.h>
#include "hosted.pb.h"

// Initializes hosted command
#define INIT_HD_COMMAND(XX) \
	HostedCommand message = HostedCommand_init_zero; \
	message.type = HostedMessageType_TypeRequest ## XX; \
	message.which_payload = HostedCommand_request ## XX ##_tag; \
	auto command = &message.payload.request ## XX;

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

	bool send(const HostedCommand& message)
	{
		bool status;

		pb_ostream_t ouput = pb_ostream_from_buffer(buffer, sizeof(buffer));
		status = pb_encode(&ouput, HostedCommand_fields, &message);

		if (!status) {
			debug_e("Encoding failed: %s\n", PB_GET_ERROR(&ouput));
			return false;
		}

		if(ouput.bytes_written > 0) {
			size_t written = stream->write(buffer, ouput.bytes_written);
			if(written != ouput.bytes_written) {
				debug_e("Unable to queue message in transport stream");
				return false;
			}
		}

		return true;
	}
private:
	ReadWriteStream* stream = nullptr;

	uint8_t buffer[128]; /* This is the buffer where we will store our message. */
};
