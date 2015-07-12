/*
 * CommandDelegate.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */

#ifndef SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_
#define SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_

#include "../Wiring/WString.h"
#include "Delegate.h"
#include "Network/TcpClient.h"
#include "WiringFrameworkIncludes.h"
#include "CommandOutput.h"

typedef Delegate<void(String commandLine  ,CommandOutput* commandOutput)> commandFunctionDelegate;

class CommandDelegate
{
	// Hashmap uses CommandDelegate() contructor when extending size
	friend class HashMap<String, CommandDelegate>;
public:
	CommandDelegate(String reqName, String reqHelp, String reqGroup, commandFunctionDelegate reqFunction);
	~CommandDelegate();

	String commandName;
	String commandHelp;
	String commandGroup;
	commandFunctionDelegate commandFunction;

private :
	CommandDelegate();

};

#endif /* SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_ */
