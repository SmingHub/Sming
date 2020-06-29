/*
 * HostedServer.h
 *
 *  Created on: Jun 29, 2020
 *      Author: slavey
 */

#pragma once

#include <SmingCore.h>
#include <pb_decode.h>
#include "hosted.pb.h"

constexpr int HOSTED_OK = 0;
constexpr int HOSTED_FAIL = -1;
constexpr int HOSTED_NO_MEM = -2;

typedef Delegate<int(HostedCommand *request, HostedCommand *response)> HostedCommandDelegate;

class HostedServer
{
public:
	void registerCommand(HostedMessageType type, HostedCommandDelegate callback)
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

		int result = HOSTED_OK;
		bool status;

		// extract the request message
		pb_istream_t input = pb_istream_from_buffer(at, length);

		status = pb_decode(&input, HostedCommand_fields, &request);
		if (!status) {
			debug_e("Decoding failed: %s\n", PB_GET_ERROR(&input));
			return HOSTED_FAIL;
		}

		// dispatch the command
		if(commands.contains(request.type) && commands[request.type] != nullptr) {
			result = commands[request.type](&request, &response);
		}
		else {
			debug_w("No command registered for type: %d", request.type);
		}

		// TODO: cleanup

		return result;

	}

private:
	HashMap<HostedMessageType, HostedCommandDelegate> commands;
};
