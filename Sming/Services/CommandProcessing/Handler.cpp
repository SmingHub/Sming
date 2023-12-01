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
		if(getVerboseMode() == VERBOSE) {
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

	if(getVerboseMode() == VERBOSE) {
		outputStream->print(getCommandPrompt());
	}
}

void Handler::registerSystemCommands()
{
	String system = F("system");
	registerCommand(Command(F("status"), F("Displays System Information"), system,
							Command::Callback(&Handler::procesStatusCommand, this)));
	registerCommand(Command(F("echo"), F("Displays command entered"), system,
							Command::Callback(&Handler::procesEchoCommand, this)));
	registerCommand(Command(F("help"), F("Displays all available commands"), system,
							Command::Callback(&Handler::procesHelpCommand, this)));
	registerCommand(Command(F("debugon"), F("Set Serial debug on"), system,
							Command::Callback(&Handler::procesDebugOnCommand, this)));
	registerCommand(Command(F("debugoff"), F("Set Serial debug off"), system,
							Command::Callback(&Handler::procesDebugOffCommand, this)));
	registerCommand(Command(F("command"), F("Use verbose/silent/prompt as command options"), system,
							Command::Callback(&Handler::processCommandOptions, this)));
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
	for(unsigned idx = 0; idx < registeredCommands.count(); idx++) {
		outputStream.print(registeredCommands.valueAt(idx).name);
		outputStream.print(" | ");
		outputStream.print(registeredCommands.valueAt(idx).group);
		outputStream.print(" | ");
		outputStream.print(registeredCommands.valueAt(idx).description);
		outputStream.print("\r\n");
	}
}

void Handler::procesStatusCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("StatusCommand entered");
	outputStream.println(_F("Sming Framework Version : " SMING_VERSION));
	outputStream.print(_F("ESP SDK version : "));
	outputStream.print(system_get_sdk_version());
	outputStream.println();
	outputStream.print(_F("Time = "));
	outputStream.print(SystemClock.getSystemTimeString());
	outputStream.println();
	outputStream.printf(_F("System Start Reason : %d\r\n"), system_get_rst_info()->reason);
}

void Handler::procesEchoCommand(String commandLine, ReadWriteStream& outputStream)
{
	debug_d("HelpCommand entered");
	outputStream.print(_F("You entered : '"));
	outputStream.print(commandLine);
	outputStream.println('\'');
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
			setVerboseMode(VERBOSE);
			outputStream.println(_F("Verbose mode selected"));
			break;
		}
		if(commandToken[1] == _F("silent")) {
			setVerboseMode(SILENT);
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
		outputStream.print(_F("Prompt set to : "));
		outputStream.print(commandToken[2]);
		outputStream.println();
		break;
	default:
		errorCommand = true;
	}
	if(errorCommand) {
		outputStream.print(_F("Unknown command : "));
		outputStream.print(commandLine);
		outputStream.println();
	}
	if(printUsage) {
		outputStream.println(_F("command usage : \r\n"));
		outputStream.println(_F("command verbose : Set verbose mode"));
		outputStream.println(_F("command silent : Set silent mode"));
		outputStream.println(_F("command prompt 'new prompt' : Set prompt to use"));
	}
}

} // namespace CommandProcessing
