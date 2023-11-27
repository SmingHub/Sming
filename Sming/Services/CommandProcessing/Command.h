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

#include <Delegate.h>
#include <Data/Stream/ReadWriteStream.h>

namespace CommandProcessing
{

/** @brief  Command delegate class */
class Command
{
public:
	/** @brief  Command delegate function
	 *  @param  commandLine Command line entered by user at CLI, including command and parameters
	 *  @param  commandOutput Pointer to the CLI print stream
	 *  @note   CommandFunctionDelegate defines the structure of a function that handles individual commands
	 *  @note   Can use standard print functions on commandOutput
	 */
	using Callback = Delegate<void(String commandLine, ReadWriteStream& commandOutput)>;


	/** Instantiate a command delegate
	*  @param  reqName Command name - the text a user types to invoke the command
	*  @param  reqHelp Help message shown by CLI "help" command
	*  @param  reqGroup The command group to which this command belongs
	*  @param  reqFunction Delegate that should be invoked (triggered) when the command is entered by a user
	*/
	Command(String reqName, String reqHelp, String reqGroup, Callback reqFunction)
		: name(reqName), description(reqHelp), group(reqGroup), callback(reqFunction)
	{
	}

	Command()
	{
	}

	String name;						 ///< Command name
	String description;						 ///< Command help
	String group;					 ///< Command group
	Callback callback; ///< Command Delegate (function that is called when command is invoked)
};

} // namespace CommandProcessing

/** @} */
