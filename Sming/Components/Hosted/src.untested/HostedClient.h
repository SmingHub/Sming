#pragma once

#include <SmingCore.h>
#include <Data/Stream/ReadWriteStream.h>
#include "HostedCommon.h"

extern void host_main_loop();

class HostedClient
{
public:
	HostedClient(ReadWriteStream* stream) : stream(stream)
	{
	}

	void setStream(ReadWriteStream* stream)
	{
		if(this->stream != nullptr) {
			debug_w("Discarding current stream!");
		}

		this->stream = stream;
	}

	bool send(HostedCommand* message)
	{
		pb_ostream_t ouput = newOutputStream();
		bool success = pb_encode_ex(&ouput, HostedCommand_fields, message, PB_ENCODE_DELIMITED);
		if(!success) {
			debug_e("Encoding failed: %s", PB_GET_ERROR(&ouput));
			return false;
		}

		stream->flush();

		return true;
	}

	/**
	 * @brief This method will block the execution until a message is detected
	 * @retval HostedCommand
	 */
	HostedCommand wait()
	{
		HostedCommand command = HostedCommand_init_zero;

		pb_istream_t input = newInputStream();
		size_t leftBytes = input.bytes_left;
		size_t totalBytes = input.bytes_left;
		bool success = false;
		do {
			stream->flush();
			success = pb_decode_ex(&input, HostedCommand_fields, &command, PB_DECODE_DELIMITED);
			host_main_loop();
		} while(!success);

		stream->seek(totalBytes - input.bytes_left);

		return command;
	}

	/**
	 * @brief This method handles incoming data
	 */
	bool onData(const char* at, size_t length)
	{
		return true;
	}

private:
	pb_istream_t newInputStream()
	{
		return pb_istream_t{
			.callback = [](pb_istream_t* stream, pb_byte_t* buf, size_t count) -> bool {
				auto source = static_cast<ReadWriteStream*>(stream->state);
				size_t read = source->readMemoryBlock(reinterpret_cast<char*>(buf), count);
				source->seek(read);

				return true;
			},
			.state = this->stream,
			.bytes_left = (size_t)this->stream->available(),
			.errmsg = nullptr,
		};
	}

	pb_ostream_t newOutputStream()
	{
		return pb_ostream_t{
			.callback = [](pb_ostream_t* stream, const pb_byte_t* buf, size_t count) -> bool {
				auto destination = static_cast<ReadWriteStream*>(stream->state);
				size_t written = destination->write(reinterpret_cast<const uint8_t*>(buf), count);

				return written == count;
			},
			.state = stream,
			.max_size = SIZE_MAX,
			.bytes_written = 0,
			.errmsg = nullptr,
		};
	}

private:
	HashMap<uint32_t, HostedCommandDelegate> responseCallbacks;
	ReadWriteStream* stream{nullptr};
};
