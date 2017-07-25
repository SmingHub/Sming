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
	if(registeredCommands != NULL) {
		delete registeredCommands;
	}
}

void CommandHandler::registerSystemCommands()
{
	registerCommand(CommandDelegate("status", "Displays System Information", "system", commandFunctionDelegate(&CommandHandler::procesStatusCommand,this)));
	registerCommand(CommandDelegate("echo", "Displays command entered", "system", commandFunctionDelegate(&CommandHandler::procesEchoCommand,this)));
	registerCommand(CommandDelegate("help", "Displays all available commands", "system", commandFunctionDelegate(&CommandHandler::procesHelpCommand,this)));
	registerCommand(CommandDelegate("debugon", "Set Serial debug on", "system", commandFunctionDelegate(&CommandHandler::procesDebugOnCommand,this)));
	registerCommand(CommandDelegate("debugoff", "Set Serial debug off", "system", commandFunctionDelegate(&CommandHandler::procesDebugOffCommand,this)));
	registerCommand(CommandDelegate("command","Use verbose/silent/prompt as command options","system", commandFunctionDelegate(&CommandHandler::processCommandOptions,this)));
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

VerboseMode CommandHandler::getVerboseMode()
{
	return verboseMode;
}

void CommandHandler::setVerboseMode(VerboseMode reqVerboseMode)
{
	verboseMode = reqVerboseMode;
}

String CommandHandler::getCommandPrompt()
{
	return currentPrompt;
}

void CommandHandler::setCommandPrompt(String reqPrompt)
{
	currentPrompt = reqPrompt;
}

char CommandHandler::getCommandEOL()
{
	return currentEOL;
}

void CommandHandler::setCommandEOL(char reqEOL)
{
	currentEOL = reqEOL;
}

String CommandHandler::getCommandWelcomeMessage()
{
	return currentWelcomeMessage;
}
void CommandHandler::setCommandWelcomeMessage(String reqWelcomeMessage)
{
	currentWelcomeMessage = reqWelcomeMessage;
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

void CommandHandler::processCommandOptions(String commandLine  ,CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ' , commandToken);
	bool errorCommand = false;
	bool printUsage = false;

	switch (numToken)
	{
		case 2 :
			if (commandToken[1] == "help")
			{
				printUsage = true;
			}
			if (commandToken[1] == "verbose")
			{
				commandHandler.setVerboseMode(VERBOSE);
				commandOutput->printf("Verbose mode selected\r\n");
				break;
			}
			if (commandToken[1] == "silent")
			{
				commandHandler.setVerboseMode(SILENT);
				commandOutput->printf("Silent mode selected\r\n");
				break;
			}
			errorCommand = true;
			break;
		case 3 :
			if (commandToken[1] != "prompt")
			{
				errorCommand = true;
				break;
			}
			commandHandler.setCommandPrompt(commandToken[2]);
			commandOutput->printf("Prompt set to : %s\r\n",commandToken[2].c_str());
			break;
		default :
			errorCommand = true;
	}
	if (errorCommand)
	{
		commandOutput->printf("Unknown command : %s\r\n",commandLine.c_str());
	}
	if (printUsage)
	{
		commandOutput->printf("command usage : \r\n\r\n");
		commandOutput->printf("command verbose : Set verbose mode\r\n");
		commandOutput->printf("command silent : Set silent mode\r\n");
		commandOutput->printf("command prompt 'new prompt' : Set prompt to use\r\n");
	}
}

CommandHandler commandHandler;
