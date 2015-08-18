/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "UdpConnection.h"
#include "../../Wiring/WString.h"

UdpConnection::UdpConnection() : onDataCallback(NULL)
{
	initialize();
}

UdpConnection::UdpConnection(UdpConnectionDataDelegate dataHandler) : onDataCallback(dataHandler)
{
	initialize();
}

UdpConnection::~UdpConnection()
{
	close();
}

void UdpConnection::initialize(udp_pcb* pcb /* = NULL*/)
{
	if (pcb == NULL)
		pcb = udp_new();
	udp = pcb;
	udp_recv(udp, staticOnReceive, (void*)this);
}

void UdpConnection::close()
{
	udp_recv(udp, NULL, NULL);
	udp_remove(udp);
	udp = NULL;
}

bool UdpConnection::listen(int port)
{
	if (udp != NULL && udp->local_port != 0)
		return false;
	else if (udp == NULL)
		initialize();

	debugf("UDP listen port %d", port);
	err_t res = udp_bind(udp, IP_ADDR_ANY, port);
	return res == ERR_OK;
}

bool UdpConnection::connect(IPAddress ip, uint16_t port)
{
	if (udp == NULL)
		initialize();

	if (udp->local_port == 0)
	{
		udp_bind(udp, IP_ADDR_ANY, 0);
		debugf("UDP LocalPort: %d", udp->local_port);
	}

	debugf("UDP connect to %s:%d", ip.toString().c_str(), port);
	err_t res = udp_connect(udp, ip, port);
	return res == ERR_OK;
}

void UdpConnection::send(const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	memcpy(p->payload, data, length);
	udp_send(udp, p);
	pbuf_free(p);
}

void UdpConnection::sendString(const char* data)
{
	send(data, strlen(data));
}

void UdpConnection::sendString(const String data)
{
	sendString(data.c_str());
}

void UdpConnection::sendTo(IPAddress remoteIP, uint16_t remotePort, const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	memcpy(p->payload, data, length);
	udp_sendto(udp, p, remoteIP, remotePort);
	pbuf_free(p);
}

void UdpConnection::sendStringTo(IPAddress remoteIP, uint16_t remotePort, const char* data)
{
	sendTo(remoteIP, remotePort, data, strlen(data));
}

void UdpConnection::sendStringTo(IPAddress remoteIP, uint16_t remotePort, const String data)
{
	sendStringTo(remoteIP, remotePort, data.c_str());
}

void UdpConnection::onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort)
{
	debugf("UDP received: %d bytes", buf->tot_len);
	if (onDataCallback)
	{
		char* data = new char[buf->tot_len + 1];
		pbuf_copy_partial(buf, data, buf->tot_len, 0);
		data[buf->tot_len] = '\0';

		onDataCallback(*this, data, buf->tot_len, remoteIP, remotePort);

		delete[] data;
	}
}

void UdpConnection::staticOnReceive(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	UdpConnection *self = (UdpConnection*)arg;
	if (self == NULL) return;

	IPAddress reip = addr != NULL ? IPAddress(*addr) : IPAddress();
	self->onReceive(p, reip, port);
	pbuf_free(p);
}
