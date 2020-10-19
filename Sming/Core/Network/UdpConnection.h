/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UdpConnection.h
 *
 ****/

#pragma once

#include <Network/IpConnection.h>

/** @defgroup   udp UDP
 *  @brief      Provides base for UDP clients or services
 *  @ingroup    networking
 *  @{
 */

class UdpConnection;

typedef Delegate<void(UdpConnection& connection, char* data, int size, IpAddress remoteIP, uint16_t remotePort)>
	UdpConnectionDataDelegate;

class UdpConnection : public IpConnection
{
public:
	UdpConnection()
	{
		initialize();
	}

	UdpConnection(UdpConnectionDataDelegate dataHandler) : onDataCallback(dataHandler)
	{
		initialize();
	}

	virtual ~UdpConnection()
	{
		close();
	}

	virtual bool listen(int port);
	virtual bool connect(IpAddress ip, uint16_t port);
	virtual void close();

	// After connect(..)
	virtual bool send(const char* data, int length);

	bool sendString(const char* data)
	{
		return send(data, strlen(data));
	}

	bool sendString(const String& data)
	{
		return send(data.c_str(), data.length());
	}

	virtual bool sendTo(IpAddress remoteIP, uint16_t remotePort, const char* data, int length);

	bool sendStringTo(IpAddress remoteIP, uint16_t remotePort, const char* data)
	{
		return sendTo(remoteIP, remotePort, data, strlen(data));
	}

	bool sendStringTo(IpAddress remoteIP, uint16_t remotePort, const String& data)
	{
		return sendTo(remoteIP, remotePort, data.c_str(), data.length());
	}

	/**
	 * @brief Sets the UDP multicast IP.
	 * @param ip
	 *
	 * @retval true when LWIP supports this operation, false otherwise
	 *
	 * @note This method works only when LWIP is compiled with LWIP_MULTICAST_TX_OPTIONS
	 */
	bool setMulticast(IpAddress ip);

	/**
	 * @brief Sets the UDP multicast Time-To-Live(TTL).
	 * @param ttl - time to live in hops.
	 *              For example if a milticast UDP packet needs to pass through two routes to reach the receiver then the TTL should be set to 2
	 *
	 * @retval true when LWIP supports this operation, false otherwise
	 *
	 * @note This method works only when LWIP is compiled with LWIP_MULTICAST_TX_OPTIONS
	 */
	bool setMulticastTtl(size_t ttl);

protected:
	virtual void onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort);

protected:
	bool initialize(udp_pcb* pcb = nullptr);
	static void staticOnReceive(void* arg, struct udp_pcb* pcb, struct pbuf* p, LWIP_IP_ADDR_T* addr, u16_t port);

protected:
	udp_pcb* udp = nullptr;
	UdpConnectionDataDelegate onDataCallback = nullptr;
};

/** @} */
