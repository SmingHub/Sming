/*
 * CommandHandler.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include "CommandHandler.h"
#include "CommandDelegate.h"

CommandHandler::CommandHandler()
{
	 registeredCommands = new HashMap<String, CommandDelegate>;
	 registerCommand(CommandDelegate("status", "Displays System Information", "system", commandFunctionDelegate(&CommandHandler::procesStatusCommand,this)));
	 registerCommand(CommandDelegate("echo", "Displays command entered", "system", commandFunctionDelegate(&CommandHandler::procesEchoCommand,this)));
	 registerCommand(CommandDelegate("help", "Displays all available commands", "system", commandFunctionDelegate(&CommandHandler::procesHelpCommand,this)));
}

CommandHandler::~CommandHandler()
{
	// TODO Auto-generated destructor stub
}

CommandDelegate CommandHandler::getCommandDelegate(String commandString)
{
	if (registeredCommands->contains(commandString))
	{
		debugf("Returning Delegate for %s \r\n",commandString.c_str());
		return (*registeredCommands)[commandString];
	}
	else
	{
		debugf("Command %s not recognized, returning NULL\r\n",commandString.c_str());
		return CommandDelegate("","","",NULL);
	}
}

bool CommandHandler::registerCommand(CommandDelegate reqDelegate)
{
	if (registeredCommands->contains(reqDelegate.commandName))
	{
		// Command already registered, don't allow  duplicates
		return false;
	}
	else
	{
		(*registeredCommands)[reqDelegate.commandName] = reqDelegate;
		return true;
	}
}

bool CommandHandler::unregisterCommand(CommandDelegate reqDelegate)
{
	if (!registeredCommands->contains(reqDelegate.commandName))
	{
		// Command not registered, cannot remove
		return false;
	}
	else
	{
		registeredCommands->remove(reqDelegate.commandName);
		(*registeredCommands)[reqDelegate.commandName] = reqDelegate;
		return true;
	}
}

void CommandHandler::procesHelpCommand(String commandLine, TcpClient* commandClient)
{
	debugf("HelpCommand entered");
	commandClient->sendString("Commands available are : \r\n");
	for (int idx = 0;idx < registeredCommands->count();idx++)
	{
		commandClient->sendString(registeredCommands->valueAt(idx).commandName);
		commandClient->sendString(" | ");
		commandClient->sendString(registeredCommands->valueAt(idx).commandGroup);
		commandClient->sendString(" | ");
		commandClient->sendString(registeredCommands->valueAt(idx).commandHelp);
		commandClient->sendString("\r\n");
	}
}

void CommandHandler::procesStatusCommand(String commandLine  ,TcpClient* commandClient)
{
	debugf("StatusCommand entered");
	char tempBuf[64];
	commandClient->sendString("System information : ESP8266 Sming Framework\r\n");
	commandClient->sendString("Sming Framework Version : 1.2.0\r\n");
	commandClient->sendString("ESP SDK version : ");
	commandClient->sendString(system_get_sdk_version());
	commandClient->sendString("\r\n");
	commandClient->sendString("Time = ");
	commandClient->sendString(SystemClock.getSystemTimeString());
	commandClient->sendString("\r\n");
	int sz = sprintf(tempBuf,"System Start Reason : %d\r\n", system_get_rst_info()->reason);
	commandClient->send(tempBuf,sz);
}

void CommandHandler::procesEchoCommand(String commandLine, TcpClient* commandClient)
{
	debugf("HelpCommand entered");
	commandClient->sendString("You entered : '");
	commandClient->sendString(commandLine);
	commandClient->sendString("'\r\n");
}

CommandHandler commandHandler;
