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

#include <WVector.h>
#include <Data/Stream/ReadWriteStream.h>
#include <Data/Stream/MemoryDataStream.h>
#include <Data/Buffer/LineBuffer.h>
#include "Command.h"

namespace CommandProcessing
{
constexpr size_t MAX_COMMANDSIZE = 64;

/** @brief  Command handler class */
class Handler
{
public:
	/**
	 * @brief  Instantiate a CommandHandler
	 */
	Handler()
	{
	}

	Handler(ReadWriteStream* stream, bool owned = true) : outputStream(stream), ownedStream(owned)
	{
	}

	Handler(const Handler& rhs) = delete;

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
		if(ownedStream) {
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

	/**
	 * @brief Process command input and return response text
	 * @param buffer Command input
	 * @param size Number of characters to process
	 * @retval String Response text
	 * @note  Do not use this method if `setOutputStream` has been called
	 */
	String processNow(const char* buffer, size_t size);

	// Command registration/de-registration methods

	/** @brief  Add a new command to the command handler
	 *  @param  command Command to register
	 *  @retval bool True on success
	 *  @note   If command already exists, it will not be replaced and function will fail.
                Call unregisterCommand first if you want to replace a command.
	 */
	bool registerCommand(const Command& command);

	/** @brief  Remove a command from the command handler
	 *  @param  command Item to remove from command handler
	 */
	bool unregisterCommand(const Command& command);

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

	/** @brief  Find command object
	 *  @param  name Command to query
	 *  @retval Command The command object matching the command
	 */
	Command getCommand(const String& name) const;

	/** @brief  Get the verbose mode
	 *  @retval bool Verbose mode
	 */
	bool isVerbose() const
	{
		return verboseMode;
	}

	/** @brief  Set the verbose mode
	 *  @param  mode Verbose mode to set
	 */
	void setVerbose(bool mode)
	{
		verboseMode = mode;
	}

	/** @brief  Get the command line prompt
	 *  @retval String The command line prompt
	 *  @note   This is what is shown on the command line before user input
	 *          Default is Sming>
	 */
	String getCommandPrompt() const;

	/** @brief  Set the command line prompt
	 *  @param  prompt The command line prompt, nullptr to reset to default
	 *  @note   This is what is shown on the command line before user input
	 *          Default is Sming>
	 */
	void setCommandPrompt(const String& prompt)
	{
		this->prompt = prompt;
	}

	/** @brief  Get the end of line character
	 *  @retval char The EOL character
	 *  @note   Only supports one EOL, unlike Windows
	 *  @deprecated Not required
	 */
	char getCommandEOL() const SMING_DEPRECATED
	{
		return '\n';
	}

	/** @brief  Set the end of line character
	 *  @param  eol The EOL character
	 *  @note   Only supports one EOL, unlike Windows
	 *  @deprecated Not required
	 */
	void setCommandEOL(char eol) SMING_DEPRECATED
	{
		(void)eol;
	}

	/** @brief  Get the welcome message
	 *  @retval String The welcome message that should be shown when clients connect
	 *  @note   For use by application
	 */
	String getCommandWelcomeMessage() const;

	/** @brief  Set the welcome message
	 *  @param  message The welcome message that should be shown when clients connect
	 *  @note   For use by application
	 *          Pass nullptr to revert to default message.
	 */
	void setCommandWelcomeMessage(const String& message)
	{
		welcomeMessage = message;
	}

private:
	Vector<CommandDef> registeredCommands;
	String prompt;
	bool verboseMode{false};
	String welcomeMessage;

	ReadWriteStream* outputStream{nullptr};
	bool ownedStream = true;
	LineBuffer<MAX_COMMANDSIZE> commandBuf;

	void processHelpCommand(String commandLine, ReadWriteStream& outputStream);
	void processStatusCommand(String commandLine, ReadWriteStream& outputStream);
	void processEchoCommand(String commandLine, ReadWriteStream& outputStream);
	void processDebugOnCommand(String commandLine, ReadWriteStream& outputStream);
	void processDebugOffCommand(String commandLine, ReadWriteStream& outputStream);
	void processCommandOptions(String commandLine, ReadWriteStream& outputStream);

	void processCommandLine(const String& cmdString);
};

} // namespace CommandProcessing

/** @} */
