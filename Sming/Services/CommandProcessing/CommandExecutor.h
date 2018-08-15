/*
 * CommandExecutor.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#ifndef SERVICES_COMMANDPROCESSING_COMMANDEXECUTOR_H_
#define SERVICES_COMMANDPROCESSING_COMMANDEXECUTOR_H_

#include "WiringFrameworkIncludes.h"
#include "Network/TcpClient.h"
#include "CommandHandler.h"
#include "CommandOutput.h"

#define MAX_COMMANDSIZE 64

class CommandExecutor {
public:
	CommandExecutor(TcpClient* cmdClient);
	CommandExecutor(Stream* reqStream);
	CommandExecutor(WebSocketConnection* reqSocket);
	~CommandExecutor();

	int executorReceive(const char* recvData, int recvSize);
	int executorReceive(char recvChar);
	int executorReceive(String recvString)
	{
		return executorReceive(recvString.c_str(), recvString.length());
	}
	void setCommandPrompt(String reqPrompt);
	void setCommandEOL(char reqEOL);

private:
	CommandExecutor();
	void processCommandLine(const String& cmdString);
	char commandBuf[MAX_COMMANDSIZE + 1];
	uint16_t commandIndex = 0;
	CommandOutput* commandOutput = nullptr;
};

#endif /* SERVICES_COMMANDPROCESSING_COMMANDEXECUTOR_H_ */
