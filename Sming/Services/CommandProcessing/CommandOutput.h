/*
 * CommandOutput.h
 *
 *  Created on: 5 jul. 2015
 *      Author: Herman
 */

#pragma once

#include "Network/TcpClient.h"
#include "Stream.h"
#include "Print.h"
#include "WiringFrameworkDependencies.h"
#include "Network/Http/Websocket/WebsocketConnection.h"

class CommandOutput : public Print
{
public:
	CommandOutput(TcpClient* reqClient);
	CommandOutput(Stream* reqStream);
	CommandOutput(WebsocketConnection* reqSocket);
	virtual ~CommandOutput();

	size_t write(uint8_t outChar);

	TcpClient* outputTcpClient = nullptr;
	Stream* outputStream = nullptr;
	WebsocketConnection* outputSocket = nullptr;
	String tempSocket = "";
};
