/*
 * CommandExecutor.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#pragma once

#include "WiringFrameworkIncludes.h"
#include "Network/TcpClient.h"
#include "CommandHandler.h"
#include "CommandOutput.h"
#include <Data/Buffer/LineBuffer.h>

#define MAX_COMMANDSIZE 64

class CommandExecutor
{
public:
	CommandExecutor(TcpClient* cmdClient);
	CommandExecutor(Stream* reqStream);
	CommandExecutor(WebsocketConnection* reqSocket);
	~CommandExecutor();

	int executorReceive(char *recvData, int recvSize);
	int executorReceive(char recvChar);
	int executorReceive(String recvString);
	void setCommandPrompt(String reqPrompt);
	void setCommandEOL(char reqEOL);

private :
	CommandExecutor();
	void processCommandLine(String cmdString);
	LineBuffer<MAX_COMMANDSIZE+1> commandBuf;
	CommandOutput* commandOutput = nullptr;
};
