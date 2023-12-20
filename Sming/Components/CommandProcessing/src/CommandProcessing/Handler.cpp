/*
 * CommandHandler.cpp
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#include <SmingVersion.h>
#include <SystemClock.h>
#include <debug_progmem.h>
#include <esp_system.h>
#include "Handler.h"

namespace CommandProcessing
{
Handler::Handler() : currentPrompt(F("Sming>")), currentWelcomeMessage(F("Welcome to the Sming CommandProcessing\r\n"))
{
}

size_t Handler::process(char recvChar)
{
	auto& output = getOutputStream();

	if(recvChar == 27) // ESC -> delete current commandLine
	{
		commandBuf.clear();
		if(isVerbose()) {
			output.println();
			output.print(getCommandPrompt());
		}
	} else if(recvChar == getCommandEOL()) {
		String command(commandBuf.getBuffer(), commandBuf.getLength());
		commandBuf.clear();
		processCommandLine(command);
	} else if(recvChar == '\b' || recvChar == 0x7f) {
		if(commandBuf.backspace()) {
			output.print(_F("\b \b"));
		}
	} else {
		if(commandBuf.addChar(recvChar) && localEcho) {
			output.print(recvChar);
		}
	}
	return 1;
}

String Handler::processNow(const char* buffer, size_t size)
{
	if(outputStream != nullptr && outputStream->getStreamType() != eSST_MemoryWritable) {
		debug_e("Cannot use this method when output stream is set");
	}

	size_t processed = process(buffer, size);
	if(processed == size) {
		String output;
		if(outputStream->moveString(output)) {
			return output;
		}
	}

	return nullptr;
}

void Handler::processCommandLine(const String& cmdString)
{
	if(cmdString.length() == 0) {
		outputStream->println();
	} else {
		debug_d("Received full Command line, size = %u,cmd = %s", cmdString.length(), cmdString.c_str());
		String cmdCommand;
		int cmdLen = cmdString.indexOf(' ');
		if(cmdLen < 0) {
			cmdCommand = cmdString;
		} else {
			cmdCommand = cmdString.substring(0, cmdLen);
		}

		debug_d("CommandExecutor : executing command %s", cmdCommand.c_str());

		Command cmdDelegate = getCommandDelegate(cmdCommand);

		if(!cmdDelegate.callback) {
			outputStream->print(_F("Command not found, cmd = '"));
			outputStream->print(cmdCommand);
			outputStream->println('\'');
		} else {
			cmdDelegate.callback(cmdString, *outputStream);
		}
	}

	if(isVerbose()) {
		outputStream->print(getCommandPrompt());
	}
}

void Handler::registerSystemCommands()
{
	String system = F("system");
	registerCommand({F("status"), F("Displays System Information"), system, {&Handler::procesStatusCommand, this}});
	registerCommand({F("echo"), F("Displays command entered"), system, {&Handler::procesEchoCommand, this}});
	registerCommand({F("help"), F("Displays all available commands"), system, {&Handler::procesHelpCommand, this}});
	registerCommand({F("debugon"), F("Set Serial debug on"), system, {&Handler::procesDebugOnCommand, this}});
	registerCommand({F("debugoff"), F("Set Serial debug off"), system, {&Handler::procesDebugOffCommand, this}});
	registerCommand({F("command"),
					 F("Use verbose/silent/prompt as command options"),
					 system,
					 {&Handler::processCommandOptions, this}});
}

Command Handler::getCommandDelegate(const String& commandString)
{
	if(registeredCommands.contains(commandString)) {
		debug_d("Returning Delegate for %s \r\n", commandString.c_str());
		return registeredCommands[commandString];
	} else {
		debug_d("Command %s not recognized, returning NULL\r\n", commandString.c_str());
		return Command("", "", "", nullptr);
	}
}

bool Handler::registerCommand(Command reqDelegate)
{
	if(registeredCommands.contains(reqDelegate.name)) {
		// Command already registered, don't allow  duplicates
		debug_d("Commandhandler duplicate command %s", reqDelegate.name.c_str());
		return false;
	} else {
		registeredCommands[reqDelegate.name] = reqDelegate;
		debug_d("Commandhandlercommand %s registered", reqDelegate.name.c_str());
		return true;
	}
}

bool Handler::unregisterCommand(Command reqDelegate)
{
	if(!registeredCommands.contains(reqDelegate.name)) {
		// Command not registered, cannot remove
		return false;
	} else {
		registeredCommands.remove(reqDelegate.name);
		//		(*registeredCommands)[reqDelegate.commandName] = reqDelegate;
		return true;
	}
}

void Handler::procesHelpCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("HelpCommand entered");
	outputStream.println(_F("Commands available are :"));
	for(auto cmd : registeredCommands) {
		outputStream << cmd->name << " | " << cmd->group << " | " << cmd->description << endl;
	}
}

void Handler::procesStatusCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("StatusCommand entered");
	outputStream << _F("Sming Framework Version : " SMING_VERSION) << endl;
	outputStream << _F("ESP SDK version : ") << system_get_sdk_version() << endl;
	outputStream << _F("Time = ") << SystemClock.getSystemTimeString() << endl;
	outputStream << _F("System Start Reason : ") << system_get_rst_info()->reason << endl;
}

void Handler::procesEchoCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("HelpCommand entered");
	outputStream << _F("You entered : '") << commandLine << '\'' << endl;
}

void Handler::procesDebugOnCommand(String commandLine, ReadWriteStream& outputStream)
{
	//	Serial.systemDebugOutput(true);
	//	outputStream.println(_F("Debug set to : On"));
}

void Handler::procesDebugOffCommand(String commandLine, ReadWriteStream& outputStream)
{
	//	Serial.systemDebugOutput(false);
	//	outputStream.println(_F("Debug set to : Off"));
}

void Handler::processCommandOptions(String commandLine, ReadWriteStream& outputStream)
{
	Vector<String> commandToken;
	int numToken = splitString(commandLine, ' ', commandToken);
	bool errorCommand = false;
	bool printUsage = false;

	switch(numToken) {
	case 2:
		if(commandToken[1] == _F("help")) {
			printUsage = true;
		}
		if(commandToken[1] == _F("verbose")) {
			setVerbose(true);
			outputStream.println(_F("Verbose mode selected"));
			break;
		}
		if(commandToken[1] == _F("silent")) {
			setVerbose(false);
			outputStream.println(_F("Silent mode selected"));
			break;
		}
		errorCommand = true;
		break;
	case 3:
		if(commandToken[1] != _F("prompt")) {
			errorCommand = true;
			break;
		}
		setCommandPrompt(commandToken[2]);
		outputStream << _F("Prompt set to : ") << commandToken[2] << endl;
		break;
	default:
		errorCommand = true;
	}
	if(errorCommand) {
		outputStream << _F("Unknown command : ") << commandLine << endl;
	}
	if(printUsage) {
		outputStream << _F("command usage :") << endl
					 << _F("command verbose : Set verbose mode") << endl
					 << _F("command silent : Set silent mode") << endl
					 << _F("command prompt 'new prompt' : Set prompt to use") << endl;
	}
}

} // namespace CommandProcessing
