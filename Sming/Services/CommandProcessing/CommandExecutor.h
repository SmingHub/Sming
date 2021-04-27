/*
 * CommandExecutor.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#pragma once

#include "CommandHandler.h"
#include "CommandOutput.h"
#include <Data/Buffer/LineBuffer.h>

#ifndef DISABLE_WIFI
#include <Network/TcpClient.h>
#endif

#define MAX_COMMANDSIZE 64

class CommandExecutor
{
public:
#ifndef DISABLE_WIFI
	CommandExecutor(TcpClient* cmdClient);
	CommandExecutor(WebsocketConnection* reqSocket);
#endif
	CommandExecutor(Stream* reqStream);
	~CommandExecutor();

	int executorReceive(char* recvData, int recvSize);
	int executorReceive(char recvChar);
	int executorReceive(const String& recvString);
	void setCommandEOL(char reqEOL);

private:
	CommandExecutor();
	void processCommandLine(const String& cmdString);
	LineBuffer<MAX_COMMANDSIZE + 1> commandBuf;
	CommandOutput* commandOutput = nullptr;
};
