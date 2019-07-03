/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UdpConnection.h
 *
 ****/

/** @defgroup   udp UDP
 *  @brief      Provides base for UDP clients or services
 *  @ingroup    networking
 *  @{
 */

#pragma once

#include "WiringFrameworkDependencies.h"
#include "IPAddress.h"

class UdpConnection;

typedef Delegate<void(UdpConnection& connection, char* data, int size, IPAddress remoteIP, uint16_t remotePort)>
	UdpConnectionDataDelegate;

class UdpConnection
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
	virtual bool connect(IPAddress ip, uint16_t port);
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

	virtual bool sendTo(IPAddress remoteIP, uint16_t remotePort, const char* data, int length);

	bool sendStringTo(IPAddress remoteIP, uint16_t remotePort, const char* data)
	{
		return sendTo(remoteIP, remotePort, data, strlen(data));
	}

	bool sendStringTo(IPAddress remoteIP, uint16_t remotePort, const String& data)
	{
		return sendTo(remoteIP, remotePort, data.c_str(), data.length());
	}

protected:
	virtual void onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort);

protected:
	bool initialize(udp_pcb* pcb = nullptr);
	static void staticOnReceive(void* arg, struct udp_pcb* pcb, struct pbuf* p, LWIP_IP_ADDR_T* addr, u16_t port);

protected:
	udp_pcb* udp = nullptr;
	UdpConnectionDataDelegate onDataCallback = nullptr;
};

/** @} */
