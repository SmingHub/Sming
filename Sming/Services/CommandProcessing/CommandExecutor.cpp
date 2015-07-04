/*
 * CommandExecutor.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include "CommandExecutor.h"

CommandExecutor::CommandExecutor(TcpClient* cmdClient)
: commandClient(cmdClient)
{
	commandClient->sendString("Welcome to Sming Command Executor\r\n");
}

CommandExecutor::~CommandExecutor()
{
}

int CommandExecutor::executorReceive(char *recvData, int recvSize)
{
	int receiveReturn = 0;
	for (int recvIdx=0;recvIdx<recvSize;recvIdx++)
	{
		receiveReturn = executorReceive(recvData[recvIdx]);
		if (receiveReturn)
		{
			break;
		}
	}
	return receiveReturn;
}

int CommandExecutor::executorReceive(char recvChar)
{
	debugf("CommandExecutor Char received\r\n" );
	if ((recvChar >= 240) || (recvChar == '\r'))
	{
		debugf("CommandExecutor : Character ignored\r\n");
	}
	else
	{
		if (recvChar == '\n')
		{
			processCommandLine(String(commandBuf));
			commandIndex = 0;
		}
		else
		{
			if (commandIndex < MAX_COMMANDSIZE)
			{
				commandBuf[commandIndex++] = recvChar;
				commandBuf[commandIndex] = 0;
			}
		}
	}
	debugf("CommandExecutor commandIndex = %d",commandIndex);
	return 0;
}

void CommandExecutor::processCommandLine(String cmdString)
{
	debugf("Received full Command line, size = %d,cmd = %s",cmdString.length(),cmdString.c_str());
	String cmdCommand;
	int cmdLen = cmdString.indexOf(' ');
	if (cmdLen == -1)
	{
		cmdCommand = cmdString;
	}
	else
	{
		cmdCommand = cmdString.substring(0,cmdLen);
	}

	debugf("CommandExecutor : executing command %s",cmdCommand.c_str());

	CommandDelegate cmdDelegate = commandHandler.getCommandDelegate(cmdCommand);

	if (!cmdDelegate.commandFunction)
	{
		commandClient->sendString("Command not found, cmd = '");
		commandClient->sendString(cmdCommand);
		commandClient->sendString("'\r\n");
	}
	else
	{
		cmdDelegate.commandFunction(cmdString.c_str(),commandClient);
	}
	commandClient->sendString("Sming>");
}


