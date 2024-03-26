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
	*  @param  name Command name - the text a user types to invoke the command
	*  @param  help Help message shown by CLI "help" command
	*  @param  group The command group to which this command belongs
	*  @param  callback Delegate that should be invoked (triggered) when the command is entered by a user
	*/
	Command(String name, String help, String group, Callback callback)
		: name(name), help(help), group(group), callback(callback)
	{
	}

	Command()
	{
	}

	explicit operator bool() const
	{
		return name;
	}

	bool operator==(const String& name) const
	{
		return name == this->name;
	}

	String name;
	String help;
	String group;
	Callback callback; ///< Command Delegate (function that is called when command is invoked)
};

} // namespace CommandProcessing

/** @} */
