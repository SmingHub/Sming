/*
 * CommandHandler.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#ifndef SERVICES_COMMANDPROCESSING_COMMANDHANDLER_H_
#define SERVICES_COMMANDPROCESSING_COMMANDHANDLER_H_

#include "../Wiring/WiringFrameworkIncludes.h"
#include "CommandDelegate.h"
#include "../Wiring/Whashmap.h"
#include "SystemClock.h"
#include <stdio.h>
#include "HardwareSerial.h"


class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();
	bool registerCommand(CommandDelegate);
	bool unregisterCommand(CommandDelegate);
	void registerSystemCommands();
	CommandDelegate getCommandDelegate(String commandString);
//	int deleteGroup(String reqGroup);

private :
	HashMap<String, CommandDelegate> *registeredCommands;
	void procesHelpCommand(String commandLine, CommandOutput* commandOutput);
	void procesStatusCommand(String commandLine, CommandOutput* commandOutput);
	void procesEchoCommand(String commandLine, CommandOutput* commandOutput);
	void procesDebugOnCommand(String commandLine, CommandOutput* commandOutput);
	void procesDebugOffCommand(String commandLine, CommandOutput* commandOutput);


};

extern CommandHandler commandHandler;

#endif /* SERVICES_COMMANDPROCESSING_COMMANDHANDLER_H_ */
