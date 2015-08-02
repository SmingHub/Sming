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
}

CommandHandler::~CommandHandler()
{
	// TODO Auto-generated destructor stub
}

void CommandHandler::registerSystemCommands()
{
	registerCommand(CommandDelegate("status", "Displays System Information", "system", commandFunctionDelegate(&CommandHandler::procesStatusCommand,this)));
	registerCommand(CommandDelegate("echo", "Displays command entered", "system", commandFunctionDelegate(&CommandHandler::procesEchoCommand,this)));
	registerCommand(CommandDelegate("help", "Displays all available commands", "system", commandFunctionDelegate(&CommandHandler::procesHelpCommand,this)));
	registerCommand(CommandDelegate("debugon", "Set Serial debug on", "system", commandFunctionDelegate(&CommandHandler::procesDebugOnCommand,this)));
	registerCommand(CommandDelegate("debugoff", "Set Serial debug off", "system", commandFunctionDelegate(&CommandHandler::procesDebugOffCommand,this)));
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
		debugf("Commandhandler duplicate command %s", reqDelegate.commandName.c_str());
		return false;
	}
	else
	{
		(*registeredCommands)[reqDelegate.commandName] = reqDelegate;
		debugf("Commandhandlercommand %s registered", reqDelegate.commandName.c_str());
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
//		(*registeredCommands)[reqDelegate.commandName] = reqDelegate;
		return true;
	}
}

void CommandHandler::procesHelpCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("HelpCommand entered");
	commandOutput->printf("Commands available are : \r\n");
	for (int idx = 0;idx < registeredCommands->count();idx++)
	{
		commandOutput->printf(registeredCommands->valueAt(idx).commandName.c_str());
		commandOutput->printf(" | ");
		commandOutput->printf(registeredCommands->valueAt(idx).commandGroup.c_str());
		commandOutput->printf(" | ");
		commandOutput->printf(registeredCommands->valueAt(idx).commandHelp.c_str());
		commandOutput->printf("\r\n");
	}
}

void CommandHandler::procesStatusCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("StatusCommand entered");
	char tempBuf[64];
	commandOutput->printf("System information : ESP8266 Sming Framework\r\n");
	commandOutput->printf("Sming Framework Version : 1.2.0\r\n");
	commandOutput->printf("ESP SDK version : ");
	commandOutput->print(system_get_sdk_version());
	commandOutput->printf("\r\n");
	commandOutput->printf("Time = ");
	commandOutput->printf(SystemClock.getSystemTimeString().c_str());
	commandOutput->printf("\r\n");
	commandOutput->printf("System Start Reason : %d\r\n", system_get_rst_info()->reason);
}

void CommandHandler::procesEchoCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("HelpCommand entered");
	commandOutput->printf("You entered : '");
	commandOutput->printf(commandLine.c_str());
	commandOutput->printf("'\r\n");
}

void CommandHandler::procesDebugOnCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(true);
	commandOutput->printf("Debug set to : On\r\n");
}

void CommandHandler::procesDebugOffCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(false);
	commandOutput->printf("Debug set to : Off\r\n");
}

CommandHandler commandHandler;
