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
#include "Handler.h"

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
	 * @retval bool See `onSend()`
	 */
	using MessageDelegate = Delegate<bool(Message& message)>;

	/**
	 * @brief Callback to be invoked with raw data (debugging, etc.)
	 * @retval bool See `onPacket()`
	 */
	using PacketDelegate = Delegate<bool(IpAddress remoteIP, uint16_t remotePort, const uint8_t* data, size_t length)>;

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
	 * @param callback Return false from callback to prevent message being passed to other clients
	 */
	void addHandler(Handler& handler)
	{
		handlers.add(&handler);
	}

	/**
	 * @brief Remove a message handler
	 * @note If there are no more handlers then consider setting a timeout and then shutting the server down.
	 */
	void removeHandler(Handler& handler)
	{
		handlers.remove(&handler);
	}

	/**
	 * @brief Set callback to be invoked before sending a message
	 * @param callback Return true from callback to actually send packet
	 */
	void onSend(MessageDelegate callback)
	{
		sendCallback = callback;
	}

	/**
	 * @brief Set callback to be invoked for raw received data, before parsing
	 * @param callback Return true from callback to actually send packet
	 */
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
	 * @brief Send an mDNS message containing questions/answers
	 * @retval bool true if message sent successfully
	 */
	bool send(Message& message);

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

	Handler::List handlers;
	MessageDelegate sendCallback;
	PacketDelegate packetCallback;
	UdpOut out;
	bool active{false};
};

extern Server server;

} // namespace mDNS
