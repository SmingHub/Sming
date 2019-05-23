/*
 * CommandHandler.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include "CommandHandler.h"
#include "CommandDelegate.h"
#include "SmingCore.h" // SMING_VERSION

#ifndef LWIP_HASH_STR
#define LWIP_HASH_STR ""
#endif

CommandHandler::CommandHandler()
	: currentPrompt(F("Sming>")), currentWelcomeMessage(F("Welcome to the Sming CommandProcessing\r\n"))
{
	registeredCommands = new HashMap<String, CommandDelegate>;
}

CommandHandler::~CommandHandler()
{
	delete registeredCommands;
}

void CommandHandler::registerSystemCommands()
{
	String system = F("system");
	registerCommand(CommandDelegate(F("status"), F("Displays System Information"), system, commandFunctionDelegate(&CommandHandler::procesStatusCommand,this)));
	registerCommand(CommandDelegate(F("echo"), F("Displays command entered"), system, commandFunctionDelegate(&CommandHandler::procesEchoCommand,this)));
	registerCommand(CommandDelegate(F("help"), F("Displays all available commands"), system, commandFunctionDelegate(&CommandHandler::procesHelpCommand,this)));
	registerCommand(CommandDelegate(F("debugon"), F("Set Serial debug on"), system, commandFunctionDelegate(&CommandHandler::procesDebugOnCommand,this)));
	registerCommand(CommandDelegate(F("debugoff"), F("Set Serial debug off"), system, commandFunctionDelegate(&CommandHandler::procesDebugOffCommand,this)));
	registerCommand(CommandDelegate(F("command"), F("Use verbose/silent/prompt as command options"), system, commandFunctionDelegate(&CommandHandler::processCommandOptions, this)));
}

CommandDelegate CommandHandler::getCommandDelegate(const String& commandString)
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
	commandOutput->println(_F("Commands available are :"));
	for (unsigned idx = 0;idx < registeredCommands->count();idx++)
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
	commandOutput->println(_F("System information : ESP8266 Sming Framework"));
	commandOutput->println(_F("Sming Framework Version : " SMING_VERSION));
	commandOutput->print(_F("ESP SDK version : "));
	commandOutput->print(system_get_sdk_version());
	commandOutput->println();
	commandOutput->printf(_F("lwIP version : %d.%d.%d(%s)\r\n"), LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR,
						  LWIP_VERSION_REVISION, LWIP_HASH_STR);
	commandOutput->print(_F("Time = "));
	commandOutput->print(SystemClock.getSystemTimeString());
	commandOutput->println();
	commandOutput->printf(_F("System Start Reason : %d\r\n"), system_get_rst_info()->reason);
}

void CommandHandler::procesEchoCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("HelpCommand entered");
	commandOutput->print(_F("You entered : '"));
	commandOutput->print(commandLine);
	commandOutput->println('\'');
}

void CommandHandler::procesDebugOnCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(true);
	commandOutput->println(_F("Debug set to : On"));
}

void CommandHandler::procesDebugOffCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(false);
	commandOutput->println(_F("Debug set to : Off"));
}

void CommandHandler::processCommandOptions(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);
	bool errorCommand = false;
	bool printUsage = false;

	switch (numToken)
	{
		case 2 :
			if (commandToken[1] == _F("help"))
			{
				printUsage = true;
			}
			if (commandToken[1] == _F("verbose"))
			{
				commandHandler.setVerboseMode(VERBOSE);
				commandOutput->println(_F("Verbose mode selected"));
				break;
			}
			if (commandToken[1] == _F("silent"))
			{
				commandHandler.setVerboseMode(SILENT);
				commandOutput->println(_F("Silent mode selected"));
				break;
			}
			errorCommand = true;
			break;
		case 3 :
			if (commandToken[1] != _F("prompt"))
			{
				errorCommand = true;
				break;
			}
			commandHandler.setCommandPrompt(commandToken[2]);
			commandOutput->print(_F("Prompt set to : "));
			commandOutput->print(commandToken[2]);
			commandOutput->println();
			break;
		default :
			errorCommand = true;
	}
	if (errorCommand)
	{
		commandOutput->print(_F("Unknown command : "));
		commandOutput->print(commandLine);
		commandOutput->println();
	}
	if (printUsage)
	{
		commandOutput->println(_F("command usage : \r\n"));
		commandOutput->println(_F("command verbose : Set verbose mode"));
		commandOutput->println(_F("command silent : Set silent mode"));
		commandOutput->println(_F("command prompt 'new prompt' : Set prompt to use"));
	}
}

CommandHandler commandHandler;
