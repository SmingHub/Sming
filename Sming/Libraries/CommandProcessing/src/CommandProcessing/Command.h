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
#include <Data/CStringArray.h>

#ifdef CMDPROC_FLASHSTRINGS
/**
 * @brief Command strings stored in single flash block for space-efficiency
 * @tparam name Command name - the text a user types to invoke the command
 * @tparam help Help message shown by CLI "help" command
 * @tparam group The command group to which this command belongs
 */
#define CMDP_STRINGS(name, help, group) FS(name "\0" help "\0" group "\0")
#else
#define CMDP_STRINGS(name, help, group) F(name), F(help), F(group)
#endif

namespace CommandProcessing
{
// Order matches CMDP_STRINGS
enum class StringIndex {
	name,
	help,
	group,
};

/**
 * @brief Command definition stored by handler
 */
struct CommandDef {
	/** @brief  Command delegate function
	 *  @param  commandLine Command line entered by user at CLI, including command and parameters
	 *  @param  commandOutput Pointer to the CLI print stream
	 *  @note   CommandFunctionDelegate defines the structure of a function that handles individual commands
	 *  @note   Can use standard print functions on commandOutput
	 */
	using Callback = Delegate<void(String commandLine, ReadWriteStream& commandOutput)>;

	operator bool() const
	{
		return strings;
	}

	bool operator==(const String& name) const
	{
		return name == get(StringIndex::name);
	}

	String get(StringIndex index) const
	{
#ifdef CMDPROC_FLASHSTRINGS
		return strings ? CStringArray(*strings)[unsigned(index)] : nullptr;
#else
		return strings[unsigned(index)];
#endif
	}

#ifdef CMDPROC_FLASHSTRINGS
	const FlashString* strings{};
#else
	CStringArray strings;
#endif

	Callback callback;
};

/** @brief  Command delegate class */
class Command : private CommandDef
{
public:
	friend class Handler;

#ifdef CMDPROC_FLASHSTRINGS
	/** Instantiate a command delegate using block of flash strings
	 *  @param  strings Block of strings produced by `CMDP_STRINGS` macro
	 *  @param  callback Delegate that should be invoked (triggered) when the command is entered by a user
	 */
	Command(const FlashString& strings, Command::Callback callback) : Command({&strings, callback})
	{
	}
#else
	/** Instantiate a command delegate using set of wiring Strings
	 *  @param  name Command name - the text a user types to invoke the command
	 *  @param  help Help message shown by CLI "help" command
	 *  @param  group The command group to which this command belongs
	 *  @param  callback Delegate that should be invoked (triggered) when the command is entered by a user
	 */
	Command(const String& name, const String& help, const String& group, Callback callback)
		: Command({nullptr, callback})
	{
		strings.reserve(name.length() + help.length() + group.length() + 3);
		strings += name;
		strings += help;
		strings += group;
	}
#endif

	Command(const CommandDef& def) : CommandDef(def), name{*this}, help{*this}, group{*this}
	{
	}

	/**
	 * @brief Helper class for accessing individual strings
	 */
	template <StringIndex index> struct StringAccessor {
		operator String() const
		{
			return def.get(index);
		}

		const CommandDef& def;
	};

	const StringAccessor<StringIndex::name> name;
	const StringAccessor<StringIndex::help> help;
	const StringAccessor<StringIndex::group> group;
};

} // namespace CommandProcessing

/** @} */
