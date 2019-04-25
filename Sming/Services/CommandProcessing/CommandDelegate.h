/*
 * CommandDelegate.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */
/** @addtogroup commandhandler
 *  @{
 */

#ifndef SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_
#define SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_

#include "WString.h"
#include "Delegate.h"
#include "Network/TcpClient.h"
#include "WiringFrameworkIncludes.h"
#include "CommandOutput.h"

/** @brief  Command delegate function
 *  @param  commandLine Command line entered by user at CLI, including command and parameters
 *  @param  commandOutput Pointer to the CLI print stream
 *  @note   commandFunctionDelegate defines the structure of a function that handles individual commands
 *  @note   Can use standard print functions on commandOutput
 */
typedef Delegate<void(String commandLine, CommandOutput* commandOutput)> commandFunctionDelegate;

/** @brief  Command delegate class */
class CommandDelegate
{
	// Hashmap uses CommandDelegate() contructor when extending size
	friend class HashMap<String, CommandDelegate>;
public:
    /** Instantiate a command delegate
     *  @param  reqName Command name - the text a user types to invoke the command
     *  @param  reqHelp Help message shown by CLI "help" command
     *  @param  reqGroup The command group to which this command belongs
     *  @param  reqFunction Delegate that should be invoked (triggered) when the command is entered by a user
     */
	CommandDelegate(String reqName, String reqHelp, String reqGroup, commandFunctionDelegate reqFunction);
	~CommandDelegate();

	String commandName; ///< Command name
	String commandHelp; ///< Command help
	String commandGroup; ///< Command group
	commandFunctionDelegate commandFunction; ///< Command Delegate (function that is called when command is invoked)

private :
	CommandDelegate();

};

/** @} */
#endif /* SERVICES_COMMANDPROCESSING_COMMANDDELEGATE_H_ */
