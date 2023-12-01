/*
 * CommandHandler.h
 *
 *  Created on: 2 jul. 2015
 *      Author: Herman
 */
/** @defgroup commandhandler Command Handler
 *  @brief  Provide command line interface
            Commands can be added to and removed from the command handler. Each command will trigger a defined Delegate.
            A welcome message may be shown when a user connects and end of line character may be defined. An automatic "help" display is available.
 * @{
 */

#pragma once

#include <WHashMap.h>
#include <Data/Stream/ReadWriteStream.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Buffer/LineBuffer.h>
#include <memory>
#include "Command.h"

namespace CommandProcessing
{

constexpr size_t MAX_COMMANDSIZE=64;

/** @brief  Verbose mode
*/


/** @brief  Command handler class */
class Handler
{
public:
	/** @brief  Verbose mode
	*/
	enum VerboseMode {
		VERBOSE, ///< Verbose mode
		SILENT   ///< Silent mode
	};


	/**
	 * @brief  Instantiate a CommandHandler
	 */
	Handler();

	Handler(ReadWriteStream* stream, bool owned = true): outputStream(stream), ownedStream(owned)
	{
	}

	Handler(const Handler&) = delete;

	~Handler()
	{
		if(ownedStream) {
			delete outputStream;
		}
	}

	// I/O methods

	/**
	 *  @brief sets the output stream
	 *  @param stream pointer to the output stream
	 *  @param owned specifies if the output stream should be deleted in this class(owned=true)
	 */
	void setOutputStream(ReadWriteStream* stream, bool owned = true)
	{
		if(outputStream != nullptr && ownedStream) {
			delete outputStream;
		}

		outputStream = stream;
		ownedStream = owned;
	}

	ReadWriteStream& getOutputStream()
	{
		if(outputStream == nullptr) {
			outputStream = new MemoryDataStream();
			ownedStream = true;
		}

		return *outputStream;
	}

	size_t process(char charToWrite);

	/** @brief  Write chars to stream
     *  @param  buffer Pointer to buffer to write to the stream
     *  @param  size Quantity of chars to write
     *  @retval size_t Quantity of chars processed
     */
	size_t process(const char* buffer, size_t size)
	{
		size_t retval = 0;
		for(size_t i = 0; i < size; i++) {
			if(process(buffer[i]) != 1) {
				break;
			}
			retval++;
		}
		return retval;
	}

	String processNow(const char* buffer, size_t size);

	// Command registration/de-registration methods

	/** @brief  Add a new command to the command handler
	 *  @param  reqDelegate Command delegate to register
	 *  @retval bool True on success
	 *  @note   If command already exists, it will not be replaced and function will fail.
                Call unregisterCommand first if you want to replace a command.
	 */
	bool registerCommand(Command reqDelegate);

	/** @brief  Remove a command from the command handler
	 *  @brief  reqDelegate Delegate to remove from command handler
	 */
	bool unregisterCommand(Command reqDelegate);

	/** @brief  Register default system commands
	 *  @note   Adds the following system commands to the command handler
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
	 *  @retval CommandDelegate The command delegate matching the command
	 */
	Command getCommandDelegate(const String& commandString);

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

private:
	HashMap<String, Command> registeredCommands;
	String currentPrompt;
#ifdef ARCH_HOST
	char currentEOL{'\n'};
#else
	char currentEOL{'\r'};
#endif
	VerboseMode verboseMode{VERBOSE};
	bool localEcho{true};
	String currentWelcomeMessage;

	ReadWriteStream* outputStream{nullptr};
	bool ownedStream = true;
	LineBuffer<MAX_COMMANDSIZE + 1> commandBuf;

	void procesHelpCommand(String commandLine, ReadWriteStream& outputStream);
	void procesStatusCommand(String commandLine, ReadWriteStream& outputStream);
	void procesEchoCommand(String commandLine, ReadWriteStream& outputStream);
	void procesDebugOnCommand(String commandLine, ReadWriteStream& outputStream);
	void procesDebugOffCommand(String commandLine, ReadWriteStream& outputStream);
	void processCommandOptions(String commandLine, ReadWriteStream& outputStream);

	void processCommandLine(const String& cmdString);
};

} // namespace CommandProcessing

/** @} */
