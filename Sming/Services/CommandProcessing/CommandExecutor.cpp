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
		commandOutput->println(_F("Welcome to the Tcp Command executor"));
	}
}

CommandExecutor::CommandExecutor(Stream* reqStream) : CommandExecutor()
{
	commandOutput = new CommandOutput(reqStream);
	if (commandHandler.getVerboseMode() != SILENT)
	{
		commandOutput->println(_F("Welcome to the Stream Command executor"));
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
		if (receiveReturn != 0)
		{
			break;
		}
	}
	return receiveReturn;
}

int CommandExecutor::executorReceive(const String& recvString)
{
	int receiveReturn = 0;
	for (unsigned recvIdx=0;recvIdx<recvString.length();recvIdx++)
	{
		receiveReturn = executorReceive(recvString[recvIdx]);
		if (receiveReturn != 0)
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
		commandBuf.clear();
		if (commandHandler.getVerboseMode() == VERBOSE)
		{
			commandOutput->println();
			commandOutput->print(commandHandler.getCommandPrompt());
		}
	}
	else if (recvChar == commandHandler.getCommandEOL())
	{
		String command(commandBuf.getBuffer(), commandBuf.getLength());
		commandBuf.clear();
		processCommandLine(command);
	}
	else if (recvChar == '\b' || recvChar == 0x7f) {
		if(commandBuf.backspace()) {
			commandOutput->print(_F("\b \b"));
		}
	}
	else
	{
		if(commandBuf.addChar(recvChar)) {
			commandOutput->print(recvChar);
		}

	}
	return 0;
}

void CommandExecutor::processCommandLine(const String& cmdString)
{
	if (cmdString.length() == 0)
	{
		commandOutput->println();
	}
	else
	{
		debugf("Received full Command line, size = %u,cmd = %s",cmdString.length(),cmdString.c_str());
		String cmdCommand;
		int cmdLen = cmdString.indexOf(' ');
		if (cmdLen < 0)
		{
			cmdCommand = cmdString;
		}
		else
		{
			cmdCommand = cmdString.substring(0, cmdLen);
		}

		debugf("CommandExecutor : executing command %s",cmdCommand.c_str());

		CommandDelegate cmdDelegate = commandHandler.getCommandDelegate(cmdCommand);

		if (!cmdDelegate.commandFunction)
		{
			commandOutput->print(_F("Command not found, cmd = '"));
			commandOutput->print(cmdCommand);
			commandOutput->println('\'');
		}
		else
		{
			cmdDelegate.commandFunction(cmdString,commandOutput);
		}
	}

	if (commandHandler.getVerboseMode() == VERBOSE)
	{
		commandOutput->print(commandHandler.getCommandPrompt());
	}
}
