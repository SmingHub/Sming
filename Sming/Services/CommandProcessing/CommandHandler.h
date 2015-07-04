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


class CommandHandler
{
public:
	CommandHandler();
	~CommandHandler();
	bool registerCommand(CommandDelegate);
	bool unregisterCommand(CommandDelegate);
	CommandDelegate getCommandDelegate(String commandString);
//	int deleteGroup(String reqGroup);

private :
	HashMap<String, CommandDelegate> *registeredCommands;
	void procesHelpCommand(String commandLine, TcpClient* commandClient);
	void procesStatusCommand(String commandLine  ,TcpClient* commandClient);
	void procesEchoCommand(String commandLine, TcpClient* commandClient);
};

extern CommandHandler commandHandler;

#endif /* SERVICES_COMMANDPROCESSING_COMMANDHANDLER_H_ */
