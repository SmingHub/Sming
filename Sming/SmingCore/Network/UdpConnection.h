/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   udp UDP
 *  @brief      Provides base for UDP clients or services
 *  @ingroup    networking
 *  @{
 */

#ifndef SMINGCORE_NETWORK_UDPCONNECTION_H_
#define SMINGCORE_NETWORK_UDPCONNECTION_H_

#include "WiringFrameworkDependencies.h"
#include "Delegate.h"
#include "IPAddress.h"

class UdpConnection;

//typedef void (*UdpConnectionDataCallback)(UdpConnection& connection, char *data, int size, IPAddress remoteIP, uint16_t remotePort);
typedef Delegate<void(UdpConnection& connection, char* data, int size, IPAddress remoteIP, uint16_t remotePort)>
	UdpConnectionDataDelegate;

class UdpConnection {
public:
	UdpConnection()
	{
		initialize();
	}

	UdpConnection(UdpConnectionDataDelegate dataHandler) : _onDataCallback(dataHandler)
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
	virtual void send(const char* data, int length);

	void sendString(const char* data)
	{
		send(data, strlen(data));
	}

	void sendString(const String& data)
	{
		send(data.c_str(), data.length());
	}

	virtual void sendTo(IPAddress remoteIP, uint16_t remotePort, const char* data, int length);

	void sendStringTo(IPAddress remoteIP, uint16_t remotePort, const char* data)
	{
		sendTo(remoteIP, remotePort, data, strlen(data));
	}

	void sendStringTo(IPAddress remoteIP, uint16_t remotePort, const String& data)
	{
		sendTo(remoteIP, remotePort, data.c_str(), data.length());
	}

protected:
	virtual void onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort);

protected:
	void initialize(udp_pcb* pcb = nullptr);
	static void staticOnReceive(void* arg, struct udp_pcb* pcb, struct pbuf* p, LWIP_IP_ADDR_T* addr, u16_t port);

protected:
	udp_pcb* _udp = nullptr;
	UdpConnectionDataDelegate _onDataCallback = nullptr;
};

/** @} */
#endif /* SMINGCORE_NETWORK_UDPCONNECTION_H_ */
