/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Server.h
 *
 * Code originally based on MrDunk's mDNS code distributed under the MIT license.
 *
 * MIT license: https://github.com/mrdunk/esp8266_mdns/blob/master/LICENCE.txt
 *
 * However, code has been essentially rewritten so Sming LGPL v3 license applies.
 *
 */

#pragma once

#include <Delegate.h>
#include <Network/UdpConnection.h>
#include "Request.h"

namespace mDNS
{
/**
 * @brief Locates mDNS services by issuing queries
 */
class Server : protected UdpConnection
{
public:
	/**
	 * @brief Callback to be invoked for each received message
	 */
	using MessageDelegate = Delegate<void(Message& message)>;

	/**
	 * @brief Callback to be invoked with raw data (debugging, etc.)
	 */
	using PacketDelegate = Delegate<void(IpAddress remoteIP, uint16_t remotePort, const uint8_t* data, size_t length)>;

	~Server();

	bool begin();

	void end();

	bool restart()
	{
		end();
		return begin();
	}

	/**
	 * @brief Set callback to be invoked for each received message
	 */
	void onMessage(MessageDelegate callback)
	{
		messageCallback = callback;
	}

	void onPacket(PacketDelegate callback)
	{
		packetCallback = callback;
	}

	/**
	 * @brief Send a multicast query
	 * @param hostname Name to find, e.g. "_googlecast._tcp.local"
	 * @param type
	 * @retval bool false if parameters failed validation or UDP request could not be sent
	 */
	bool search(const String& hostname, ResourceType type = ResourceType::PTR);

	/**
	 * @brief Send an mDNS request containing questions/answers
	 * @retval bool true if request sent successfully
	 */
	bool send(Request& request);

protected:
	void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;

private:
	/*
	 * Need a separate UDP connection for sending requests
	 */
	class UdpOut : public UdpConnection
	{
	protected:
		void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort) override;
	};

	MessageDelegate messageCallback;
	PacketDelegate packetCallback;
	UdpOut out;
	bool active{false};
};

extern Server server;

} // namespace mDNS
