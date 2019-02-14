/*
 * CommandExecutor.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include "CommandExecutor.h"
#include "HardwareSerial.h"

CommandExecutor::CommandExecutor()
{
	commandHandler.registerSystemCommands();
}

CommandExecutor::CommandExecutor(TcpClient* cmdClient) : CommandExecutor()
{
	commandOutput = new CommandOutput(cmdClient);
	if (commandHandler.getVerboseMode() != SILENT)
	{
		commandOutput->print(_F("Welcome to the Tcp Command executor\r\n"));
	}
}

CommandExecutor::CommandExecutor(Stream* reqStream) : CommandExecutor()
{
	commandOutput = new CommandOutput(reqStream);
	if (commandHandler.getVerboseMode() != SILENT)
	{
		commandOutput->print(_F("Welcome to the Stream Command executor\r\n"));
	}
}

CommandExecutor::CommandExecutor(WebsocketConnection* reqSocket)
{
	commandOutput = new CommandOutput(reqSocket);
	if (commandHandler.getVerboseMode() != SILENT)
	{
		reqSocket->sendString(_F("Welcome to the Websocket Command Executor"));
	}

}

CommandExecutor::~CommandExecutor()
{
	delete commandOutput;
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

int CommandExecutor::executorReceive(String recvString)
{
	int receiveReturn = 0;
	for (unsigned recvIdx=0;recvIdx<recvString.length();recvIdx++)
	{
		receiveReturn = executorReceive(recvString[recvIdx]);
		if (receiveReturn)
		{
			break;
		}
	}
	return receiveReturn;
}

int CommandExecutor::executorReceive(char recvChar)
{
	if (recvChar == 27) // ESC -> delete current commandLine
	{
		commandIndex = 0;
		commandBuf[commandIndex] = 0;
		if (commandHandler.getVerboseMode() == VERBOSE)
		{
			commandOutput->print("\r\n");
			commandOutput->print(commandHandler.getCommandPrompt());
		}
	}
	else if (recvChar == commandHandler.getCommandEOL())
	{

		processCommandLine(String(commandBuf));
		commandIndex = 0;
	}
	else
	{
		if ((commandIndex < MAX_COMMANDSIZE) && (isprint(recvChar)))
		{
			commandBuf[commandIndex++] = recvChar;
			commandBuf[commandIndex] = 0;
		}

	}
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
		commandOutput->print(_F("Command not found, cmd = '"));
		commandOutput->print(cmdCommand);
		commandOutput->print(_F("'\r\n"));
	}
	else
	{
		cmdDelegate.commandFunction(cmdString.c_str(),commandOutput);
	}
	if (commandHandler.getVerboseMode() == VERBOSE)
	{
		commandOutput->printf(commandHandler.getCommandPrompt().c_str());
	}
}
