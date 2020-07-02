/*
 * HostedServer.h
 *
 *  Created on: Jun 29, 2020
 *      Author: slavey
 */

#pragma once

#include <SmingCore.h>
#include <Data/Buffer/CircularBuffer.h>
#include <HostedCommon.h>

class HostedServer
{
public:
	HostedServer(size_t storageSize = 1024): storage(new CircularBuffer(storageSize))
    {

    }

	void registerCommand(uint32_t type, HostedCommandDelegate callback)
	{
		commands[type] = callback;
	}

	/**
	 * @brief Process incoming commands
	 * @return number of processed bytes
	 */
	int process(const uint8_t *at, size_t length)
	{
		HostedCommand request = HostedCommand_init_zero;
		HostedCommand response = HostedCommand_init_zero;

		if (at == nullptr || length < 0) {
			debug_e("empty buffer");
			return HOSTED_FAIL;
		}

		size_t written = storage->write(at, length);
		if(written != length) {
			// Not enough space to store the message...
			return HOSTED_NO_MEM;
		}

		int result = HOSTED_OK;
		bool success;

		pb_istream_t input = newInputStream();
		size_t leftBytes = input.bytes_left;
		do {
			success = pb_decode_ex(&input, HostedCommand_fields, &request, PB_DECODE_DELIMITED);
			if (!(success && request.id)) {
				Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&input));
				storage->seek(storage->available()- leftBytes);
				break;
			}

			// dispatch the command
			if(!(commands.contains(request.which_payload) && commands[request.which_payload] != nullptr)) {
				debug_w("No command registered for type: %d", request.which_payload);
				continue;
			}

			result = commands[request.which_payload](&request, &response);

			// TODO: process the response

			// and send it back...
			leftBytes = input.bytes_left;

		} while(input.bytes_left && success);

		return result;

	}

private:
	pb_istream_t newInputStream()
	{
	    pb_istream_t stream;
	    stream.callback = [](pb_istream_t *stream, pb_byte_t *buf, size_t count) -> bool {
	    	CircularBuffer* source = (CircularBuffer* )stream->state;
	    	int read = source->readBytes((char *)buf, count);
	    	stream->bytes_left = source->available() - read;

	    	return true;
	    };
	    stream.state = (void*)storage;
	    stream.bytes_left = storage->available();
	    stream.errmsg = nullptr;

	    return stream;
	}


private:
	CircularBuffer* storage = nullptr;
	HashMap<uint32_t, HostedCommandDelegate> commands;
};
