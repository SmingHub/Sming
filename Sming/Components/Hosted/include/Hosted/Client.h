#pragma once

#ifndef ARCH_HOST
#error "Hosted::Client can be used only on the Host architecture!"
#endif

#include <Stream.h>
#include <simpleRPC.h>
#include <memory>
#include <hostlib/hostlib.h>

namespace Hosted
{
class Client
{
public:
	Client(Stream& stream) : stream(stream)
	{
	}

	template <typename... Args> bool send(const char* functionName, Args... args)
	{
		uint8_t functionId = getFunctionId(functionName);

		rpcPrint(stream, functionId);
		rpcPrint(stream, args...);

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

		std::shared_ptr<uint8_t> data(new uint8_t[neededBytes]);
		// 		std::unique_ptr<uint8_t> data = std::make_unique<uint8_t>(neededBytes);
		//		uint8_t* data = new uint8_t[neededBytes];
		stream.readBytes(reinterpret_cast<char*>(&data), neededBytes);

		return *(reinterpret_cast<R*>(&data));
	}

	uint8_t getFunctionId(const char* name)
	{
		// TODO: get the id corresponding to this function
		return 0;
	}

private:
	Stream& stream;
};

} // namespace Hosted
