/*
 * CommandDelegate.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */
/** @addtogroup commandhandler
 *  @{
 */

#pragma once

#include <WString.h>
#include <WHashMap.h>
#include "CommandOutput.h"

/** @brief  Command delegate function
 *  @param  commandLine Command line entered by user at CLI, including command and parameters
 *  @param  commandOutput Pointer to the CLI print stream
 *  @note   CommandFunctionDelegate defines the structure of a function that handles individual commands
 *  @note   Can use standard print functions on commandOutput
 */
using CommandFunctionDelegate = Delegate<void(String commandLine, CommandOutput* commandOutput)>;

/** @brief  Command delegate class */
class CommandDelegate
{
public:
	/** Instantiate a command delegate
	*  @param  reqName Command name - the text a user types to invoke the command
	*  @param  reqHelp Help message shown by CLI "help" command
	*  @param  reqGroup The command group to which this command belongs
	*  @param  reqFunction Delegate that should be invoked (triggered) when the command is entered by a user
	*/
	CommandDelegate(String reqName, String reqHelp, String reqGroup, CommandFunctionDelegate reqFunction)
		: commandName(reqName), commandHelp(reqHelp), commandGroup(reqGroup), commandFunction(reqFunction)
	{
	}

	CommandDelegate()
	{
	}

	String commandName;						 ///< Command name
	String commandHelp;						 ///< Command help
	String commandGroup;					 ///< Command group
	CommandFunctionDelegate commandFunction; ///< Command Delegate (function that is called when command is invoked)
};

/** @} */
