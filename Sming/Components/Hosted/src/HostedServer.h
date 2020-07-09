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

typedef Delegate <bool(const uint8_t* data, size_t size)> HostedTransferDelegate;

class HostedServer
{
public:
	HostedServer(size_t storageSize = 1024): inputBuffer(new CircularBuffer(storageSize))
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

		size_t written = inputBuffer->write(at, length);
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
			if (!(success && request.which_payload)) {
				Serial.printf("Decoding failed: %s\n", PB_GET_ERROR(&input));
				inputBuffer->seek(inputBuffer->available()- leftBytes);
				break;
			}

			leftBytes = input.bytes_left;

			// dispatch the command
			if(!(commands.contains(request.which_payload) && commands[request.which_payload] != nullptr)) {
				debug_w("No command registered for type: %d", request.which_payload);
				continue;
			}

			result = commands[request.which_payload](&request, &response);
			if(result != HOSTED_OK) {
				break;
			}

			// process the response
			if(response.which_payload && !send(&response)) {
				result = HOSTED_FAIL;
				break;
			}
		} while(input.bytes_left && success);

		return result;
	}

	bool send(HostedCommand *message)
	{
		pb_ostream_t ouput = newOutputStream();
		bool success = pb_encode_ex(&ouput, HostedCommand_fields, message, PB_ENCODE_DELIMITED);
		if (!success) {
			debug_e("Encoding failed: %s\n", PB_GET_ERROR(&ouput));
			return false;
		}

		return true;
	}

	bool transfer(HostedTransferDelegate callback)
	{
		uint8_t buf[512];
		while(outputBuffer.available() > 0) {
			int read = outputBuffer.readMemoryBlock((char *)buf, 512);
			outputBuffer.seek(read);
			if(!callback(buf, read)) {
				return false;
			}

			if(read < 1024) {
				break;
			}
		}
		return true;
	}

private:
	pb_istream_t newInputStream()
	{
	    pb_istream_t stream;
	    stream.callback = [](pb_istream_t *stream, pb_byte_t *buf, size_t count) -> bool {
	    	CircularBuffer* source = (CircularBuffer* )stream->state;
			size_t read = source->readMemoryBlock((char *)buf, count);
			source->seek(read);

	    	return true;
	    };
	    stream.state = (void*)inputBuffer;
	    stream.bytes_left = inputBuffer->available();
	    stream.errmsg = nullptr;

	    return stream;
	}

	pb_ostream_t newOutputStream()
	{
		pb_ostream_t outputStream;
		outputStream.callback = [](pb_ostream_t *stream, const pb_byte_t *buf, size_t count) -> bool {
			CircularBuffer* destination = (CircularBuffer* )stream->state;
			size_t written = destination->write((const uint8_t *)buf, count);

			return (written == count);
		};
		outputStream.state = (void*)&this->outputBuffer;
		outputStream.max_size = SIZE_MAX;
		outputStream.bytes_written = 0;
		outputStream.errmsg = nullptr;

		return outputStream;
	}


private:
	CircularBuffer* inputBuffer = nullptr;
	CircularBuffer outputBuffer = CircularBuffer(1024);
	HashMap<uint32_t, HostedCommandDelegate> commands;
};
