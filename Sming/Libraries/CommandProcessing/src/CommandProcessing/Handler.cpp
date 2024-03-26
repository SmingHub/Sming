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
		processCommandLine(String(commandBuf));
		commandBuf.clear();
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
		debug_d("Received full Command line, size = %u, cmd = '%s'", cmdString.length(), cmdString.c_str());
		String name;
		int cmdLen = cmdString.indexOf(' ');
		if(cmdLen < 0) {
			name = cmdString;
		} else {
			name = cmdString.substring(0, cmdLen);
		}

		debug_d("CommandExecutor : executing command '%s'", name.c_str());

		Command cmd = getCommand(name);
		if(!cmd) {
			*outputStream << _F("Command '") << name << _F("' not found.") << endl;
		} else if(cmd.callback) {
			cmd.callback(cmdString, *outputStream);
		} else {
			*outputStream << _F("Command '") << name << _F("' has no callback.") << endl;
		}
	}

	if(isVerbose()) {
		outputStream->print(getCommandPrompt());
	}
}

void Handler::registerSystemCommands()
{
	String system = F("system");
	registerCommand({F("status"), F("Displays System Information"), system, {&Handler::processStatusCommand, this}});
	registerCommand({F("echo"), F("Displays command entered"), system, {&Handler::processEchoCommand, this}});
	registerCommand({F("help"), F("Displays all available commands"), system, {&Handler::processHelpCommand, this}});
	registerCommand({F("debugon"), F("Set Serial debug on"), system, {&Handler::processDebugOnCommand, this}});
	registerCommand({F("debugoff"), F("Set Serial debug off"), system, {&Handler::processDebugOffCommand, this}});
	registerCommand({F("command"),
					 F("Use verbose/silent/prompt as command options"),
					 system,
					 {&Handler::processCommandOptions, this}});
}

Command Handler::getCommand(const String& name) const
{
	int i = registeredCommands.indexOf(name);
	if(i >= 0) {
		debug_d("[CH] Returning Delegate for '%s'", name.c_str());
		return registeredCommands[i];
	}

	debug_d("[CH] Command %s not recognized", name.c_str());
	return Command();
}

bool Handler::registerCommand(const Command& command)
{
	int i = registeredCommands.indexOf(command.name);
	if(i >= 0) {
		// Command already registered, don't allow  duplicates
		debug_d("[CH] Duplicate command %s", command.name.c_str());
		return false;
	}

	registeredCommands.add(command);
	debug_d("[CH] Command '%s' registered", command.name.c_str());
	return true;
}

bool Handler::unregisterCommand(const Command& command)
{
	int i = registeredCommands.indexOf(command.name);
	if(i < 0) {
		// Command not registered, cannot remove
		return false;
	}

	registeredCommands.remove(i);
	return true;
}

void Handler::processHelpCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("HelpCommand entered");
	outputStream.println(_F("Commands available are :"));
	for(auto& cmd : registeredCommands) {
		outputStream << cmd.name << " | " << cmd.group << " | " << cmd.help << endl;
	}
}

void Handler::processStatusCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("StatusCommand entered");
	outputStream << _F("Sming Framework Version : " SMING_VERSION) << endl;
	outputStream << _F("ESP SDK version : ") << system_get_sdk_version() << endl;
	outputStream << _F("Time = ") << SystemClock.getSystemTimeString() << endl;
	outputStream << _F("System Start Reason : ") << system_get_rst_info()->reason << endl;
}

void Handler::processEchoCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("HelpCommand entered");
	outputStream << _F("You entered : '") << commandLine << '\'' << endl;
}

void Handler::processDebugOnCommand(String commandLine, ReadWriteStream& outputStream)
{
	//	Serial.systemDebugOutput(true);
	//	outputStream.println(_F("Debug set to : On"));
}

void Handler::processDebugOffCommand(String commandLine, ReadWriteStream& outputStream)
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
