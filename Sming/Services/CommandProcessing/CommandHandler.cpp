/*
 * CommandHandler.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 *
 * @todo [mikee47] Clunky, needs a lot of work.
 */

#include "CommandHandler.h"
#include "CommandDelegate.h"
#include "version.h"
#include "HardwareSerial.h"
#include "SystemClock.h"

#ifndef LWIP_HASH_STR
#define LWIP_HASH_STR ""
#endif

CommandHandler::CommandHandler()
{
	_currentPrompt = _F("Sming>");
	_currentWelcomeMessage = _F("Welcome to the Sming CommandProcessing\r\n");
}

void CommandHandler::registerSystemCommands()
{
	registerCommand(CommandDelegate("status", "Displays System Information", "system",
									commandFunctionDelegate(&CommandHandler::processStatusCommand, this)));
	registerCommand(CommandDelegate("echo", "Displays command entered", "system",
									commandFunctionDelegate(&CommandHandler::processEchoCommand, this)));
	registerCommand(CommandDelegate("help", "Displays all available commands", "system",
									commandFunctionDelegate(&CommandHandler::processHelpCommand, this)));
	registerCommand(CommandDelegate("debugon", "Set Serial debug on", "system",
									commandFunctionDelegate(&CommandHandler::processDebugOnCommand, this)));
	registerCommand(CommandDelegate("debugoff", "Set Serial debug off", "system",
									commandFunctionDelegate(&CommandHandler::processDebugOffCommand, this)));
	registerCommand(CommandDelegate("command", "Use verbose/silent/prompt as command options", "system",
									commandFunctionDelegate(&CommandHandler::processCommandOptions, this)));
}

CommandDelegate CommandHandler::getCommandDelegate(const String& commandString)
{
	if (_registeredCommands.contains(commandString)) {
		debugf("Returning Delegate for %s \r\n", commandString.c_str());
		return _registeredCommands[commandString];
	}
	else {
		debugf("Command %s not recognized, returning NULL\r\n", commandString.c_str());
		return CommandDelegate();
	}
}

bool CommandHandler::registerCommand(CommandDelegate reqDelegate)
{
	if (_registeredCommands.contains(reqDelegate.commandName)) {
		// Command already registered, don't allow  duplicates
		debugf("Commandhandler duplicate command %s", reqDelegate.commandName.c_str());
		return false;
	}
	else {
		_registeredCommands[reqDelegate.commandName] = reqDelegate;
		debugf("Commandhandlercommand %s registered", reqDelegate.commandName.c_str());
		return true;
	}
}

bool CommandHandler::unregisterCommand(CommandDelegate reqDelegate)
{
	if (!_registeredCommands.contains(reqDelegate.commandName)) {
		// Command not registered, cannot remove
		return false;
	}
	else {
		_registeredCommands.remove(reqDelegate.commandName);
		//		_registeredCommands[reqDelegate.commandName] = reqDelegate;
		return true;
	}
}

void CommandHandler::processHelpCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("HelpCommand entered");
	commandOutput->printf(_F("Commands available are : \r\n"));
	for (unsigned idx = 0; idx < _registeredCommands.count(); idx++) {
		auto& value = _registeredCommands.valueAt(idx);
		commandOutput->print(value.commandName);
		commandOutput->print(" | ");
		commandOutput->print(value.commandGroup);
		commandOutput->print(" | ");
		commandOutput->print(value.commandHelp);
		commandOutput->print("\r\n");
	}
}

void CommandHandler::processStatusCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("StatusCommand entered");
	char tempBuf[64];
	commandOutput->print(_F("System information : ESP8266 Sming Framework\r\n"));
	commandOutput->print(_F("Sming Framework Version : " SMING_VERSION "\r\n"));
	commandOutput->print(_F("ESP SDK version : "));
	commandOutput->print(system_get_sdk_version());
	commandOutput->print("\r\n");
	commandOutput->printf(_F("lwIP version : %d.%d.%d(%s)\n"), LWIP_VERSION_MAJOR, LWIP_VERSION_MINOR,
						  LWIP_VERSION_REVISION, LWIP_HASH_STR);
	commandOutput->print(_F("Time = "));
	commandOutput->print(SystemClock.getSystemTimeString());
	commandOutput->print("\r\n");
	commandOutput->printf(_F("System Start Reason : %d\r\n"), system_get_rst_info()->reason);
}

void CommandHandler::processEchoCommand(String commandLine, CommandOutput* commandOutput)
{
	debugf("HelpCommand entered");
	commandOutput->print(_F("You entered : '"));
	commandOutput->print(commandLine);
	commandOutput->print(_F("'\r\n"));
}

void CommandHandler::processDebugOnCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(true);
	commandOutput->print(_F("Debug set to : On\r\n"));
}

void CommandHandler::processDebugOffCommand(String commandLine, CommandOutput* commandOutput)
{
	Serial.systemDebugOutput(false);
	commandOutput->print(_F("Debug set to : Off\r\n"));
}

void CommandHandler::processCommandOptions(String commandLine, CommandOutput* commandOutput)
{
	Vector<String> commandToken;
	int numToken = commandLine.split(' ', commandToken);
	bool errorCommand = false;
	bool printUsage = false;

	switch (numToken) {
	case 2:
		if (commandToken[1] == _F("help"))
			printUsage = true;

		if (commandToken[1] == _F("verbose")) {
			commandHandler.setVerboseMode(VERBOSE);
			commandOutput->print(_F("Verbose mode selected\r\n"));
			break;
		}

		if (commandToken[1] == _F("silent")) {
			commandHandler.setVerboseMode(SILENT);
			commandOutput->print(_F("Silent mode selected\r\n"));
			break;
		}
		errorCommand = true;
		break;

	case 3:
		if (commandToken[1] != _F("prompt")) {
			errorCommand = true;
			break;
		}
		commandHandler.setCommandPrompt(commandToken[2]);
		commandOutput->print(_F("Prompt set to : "));
		commandOutput->print(commandToken[2]);
		commandOutput->print("\r\n");
		break;

	default:
		errorCommand = true;
	}

	if (errorCommand) {
		commandOutput->print(_F("Unknown command : "));
		commandOutput->print(commandLine);
		commandOutput->print("\r\n");
	}

	if (printUsage) {
		commandOutput->print(_F("command usage : \r\n\r\n"));
		commandOutput->print(_F("command verbose : Set verbose mode\r\n"));
		commandOutput->print(_F("command silent : Set silent mode\r\n"));
		commandOutput->print(_F("command prompt 'new prompt' : Set prompt to use\r\n"));
	}
}

CommandHandler commandHandler;
