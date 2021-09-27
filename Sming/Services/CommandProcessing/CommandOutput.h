/*
 * CommandOutput.h
 *
 *  Created on: 5 jul. 2015
 *      Author: Herman
 */

#pragma once

#include <Stream.h>
#include <Print.h>

#ifndef DISABLE_NETWORK
#include <Network/TcpClient.h>
#include <Network/Http/Websocket/WebsocketConnection.h>
#endif

class CommandOutput : public Print
{
public:
#ifndef DISABLE_NETWORK
	CommandOutput(TcpClient* reqClient) : outputTcpClient(reqClient)
	{
	}

	CommandOutput(WebsocketConnection* reqSocket) : outputSocket(reqSocket)
	{
	}

#endif

	CommandOutput(Stream* reqStream);
	virtual ~CommandOutput();

	size_t write(uint8_t outChar);

#ifndef DISABLE_NETWORK
	TcpClient* outputTcpClient = nullptr;
	WebsocketConnection* outputSocket = nullptr;
#endif
	Stream* outputStream = nullptr;
	String tempSocket = "";
};
