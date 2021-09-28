/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Client.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#ifndef ARCH_HOST
#error "Hosted::Client can be used only on the Host architecture!"
#endif

#include <Stream.h>
#include <WHashMap.h>
#include <WString.h>
#include <simpleRPC.h>
#include <simpleRPC/parser.h>
#include <hostlib/emu.h>
#include <hostlib/hostmsg.h>

using namespace simpleRPC;

namespace Hosted
{
constexpr int COMMAND_NOT_FOUND = -1;

class Client
{
public:
	using RemoteCommands = HashMap<String, uint8_t>;

	Client(Stream& stream) : stream(stream)
	{
	}

	/**
	 * @brief Method to send commands to the remote server
	 * @param functionName
	 * @param variable arguments
	 *
	 * @retval true on success
	 */
	template <typename... Args> bool send(const char* functionName, Args... args)
	{
		uint8_t functionId = getFunctionId(functionName);

		rpcPrint(stream, functionId, args...);

		return true;
	}

	/**
	 * @brief This method will block the execution until a message is detected
	 * @retval HostedCommand
	 */
	template <typename R> R wait()
	{
		size_t neededBytes = sizeof(R);

		while(stream.available() < int(neededBytes)) {
			stream.flush();
			host_main_loop();
		}

		R result{};
		stream.readBytes(reinterpret_cast<char*>(&result), sizeof(result));
		return result;
	}

	/**
	 * @brief Fetches a list of commands supported on the RPC server and gives back the id of the desired command
	 * @param name command name to query
	 * @retval -1 if not found. Otherwise the id of the function
	 */
	int getFunctionId(const char* name)
	{
		if(fetchCommands) {
			getRemoteCommands();
		}

		int id = commands.indexOf(name);
		if(id < 0) {
			return COMMAND_NOT_FOUND;
		}

		return commands[name];
	}

	/**
	 * @brief Gets list of remote command names and their ids
	 * @retval true on success, false otherwise
	 */
	bool getRemoteCommands()
	{
		host_debug_i("Getting remote RPC commands \033[5m...\033[0m");

		uint8_t head = 0xff;
		stream.write(&head, 1);
		char buffer[512];
		ParserSettings settings;
		settings.startMethods = ParserSettings::SimpleMethod(&Client::startMethods, this);
		settings.startMethod = ParserSettings::SimpleMethod(&Client::startMethod, this);
		settings.methodName = ParserSettings::CharMethod(&Client::methodName, this);
		settings.endMethod = ParserSettings::SimpleMethod(&Client::endMethod, this);
		settings.endMethods = ParserSettings::SimpleMethod(&Client::endMethods, this);
		settings.state = ParserState::ready;

		do {
			stream.flush();
			host_main_loop();

			size_t length = stream.readBytes(buffer, 512);
			if(!length) {
				continue;
			}

			ParserResult result = parse(settings, buffer, length);
			if(result == ParserResult::finished) {
				break;
			}

			if(result == ParserResult::error) {
				host_debug_e("Invalid header");
				return false;
			}
		} while(true);

		host_debug_i("Connected. Starting application");

		return true;
	}

private:
	Stream& stream;
	bool fetchCommands{true};
	RemoteCommands commands;
	uint8_t methodPosition = 0;
	String parsedCommand;

	void startMethods()
	{
		methodPosition = 0;
		commands.clear();
	}

	void startMethod()
	{
		parsedCommand = "";
	}

	void methodName(char ch)
	{
		parsedCommand += ch;
	}

	void endMethod()
	{
		commands[parsedCommand] = methodPosition++;
	}

	void endMethods()
	{
		fetchCommands = false;
	}
};

} // namespace Hosted
