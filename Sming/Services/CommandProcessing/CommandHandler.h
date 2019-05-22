/*
 * CommandHandler.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */
/** @defgroup commandhandler Command Handler
 *  @brief  Provide command line interface

            Command handler provides a common command line interface. CLI is available for the following remote access methods:
            - Serial
            - Telnet
            - Websockets

            By default, CLI is disabled. Enable CLI by calling "commandProcessing" on the appropriate access class object, e.g.

                Serial.commandProcessing(true)

            Commands can be added to and removed from the command handler. Each command will trigger a defined Delegate.

            A welcome message may be shown when a user connects and end of line character may be defined. An automatic "help" display is available.
 * @{
 */

#pragma once

#include "WiringFrameworkIncludes.h"
#include "CommandDelegate.h"
#include "WHashMap.h"
#include "SystemClock.h"
#include <stdio.h>
#include "HardwareSerial.h"

/** @brief  Verbose mode
*/
typedef enum {
    VERBOSE,    ///< Verbose mode
    SILENT      ///< Silent mode
    } VerboseMode;

/** @brief  Command handler class */
class CommandHandler
{
public:
    /** @brief  Instantiate a CommandHandler
    */
	CommandHandler();
	~CommandHandler();

	/** @brief  Add a new command to the command handler
	 *  @param  reqDelegate Command delegate to register
	 *  @retval bool True on success
	 *  @note   If command already exists, it will not be replaced and function will fail.
                Call unregisterCommand first if you want to replace a command.
	 */
	bool registerCommand(CommandDelegate reqDelegate);

    /** @brief  Remove a command from the command handler
     *  @brief  reqDelegate Delegate to remove from command handler
     */
	bool unregisterCommand(CommandDelegate reqDelegate);

    /** @brief  Register default system commands
     *  @note   Adds the following system commmands to the command handler
     *          - status
     *          - echo
     *          - help
     *          - debugon
     *          - debugoff
     *          - command
     */
	void registerSystemCommands();

	/** @brief  Get the command delegate for a command
	 *  @param  commandString Command to query
	 *  @retval CommandDelegate The command delegate matchin the command
	 */
	CommandDelegate getCommandDelegate(const String& commandString);

	/** @brief  Get the verbose mode
	 *  @retval VerboseMode Verbose mode
	 */
	VerboseMode getVerboseMode()
	{
		return verboseMode;
	}

	/** @brief  Set the verbose mode
	 *  @param  reqVerboseMode Verbose mode to set
	 */
	void setVerboseMode(VerboseMode reqVerboseMode)
	{
		verboseMode = reqVerboseMode;
	}

	/** @brief  Get the command line prompt
	 *  @retval String The command line prompt
	 *  @note   This is what is shown on the command line before user input
	 *          Default is Sming>
	 */
	String getCommandPrompt()
	{
		return currentPrompt;
	}

	/** @brief  Set the command line prompt
	 *  @param  reqPrompt The command line prompt
	 *  @note   This is what is shown on the command line before user input
	 *          Default is Sming>
	 */
	void setCommandPrompt(const String& reqPrompt)
	{
		currentPrompt = reqPrompt;
	}

	/** @brief  Get the end of line character
	 *  @retval char The EOL character
	 *  @note   Only supports one EOL, unlike Windows
	 */
	char getCommandEOL()
	{
		return currentEOL;
	}

	/** @brief  Set the end of line character
	 *  @param  reqEOL The EOL character
	 *  @note   Only supports one EOL, unlike Windows
	 */
	void setCommandEOL(char reqEOL)
	{
		currentEOL = reqEOL;
	}

	/** @brief  Get the welcome message
	 *  @retval String The welcome message that is shown when clients connect
	 *  @note   Only if verbose mode is enabled
	 */
	String getCommandWelcomeMessage()
	{
		return currentWelcomeMessage;
	}

	/** @brief  Set the welcome message
	 *  @param  reqWelcomeMessage The welcome message that is shown when clients connect
	 *  @note   Only if verbose mode is enabled
	 */
	void setCommandWelcomeMessage(const String& reqWelcomeMessage)
	{
		currentWelcomeMessage = reqWelcomeMessage;
	}


//	int deleteGroup(String reqGroup);

private :
	HashMap<String, CommandDelegate> *registeredCommands;
	void procesHelpCommand(String commandLine, CommandOutput* commandOutput);
	void procesStatusCommand(String commandLine, CommandOutput* commandOutput);
	void procesEchoCommand(String commandLine, CommandOutput* commandOutput);
	void procesDebugOnCommand(String commandLine, CommandOutput* commandOutput);
	void procesDebugOffCommand(String commandLine, CommandOutput* commandOutput);
	void processCommandOptions(String commandLine  ,CommandOutput* commandOutput);

	VerboseMode verboseMode = VERBOSE;
	String currentPrompt;
	char currentEOL = '\r';
	String currentWelcomeMessage;
};

/** @brief  Global instance of CommandHandler */
extern CommandHandler commandHandler;

/** @} */
