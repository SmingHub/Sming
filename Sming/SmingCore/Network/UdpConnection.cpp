/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "UdpConnection.h"
#include "WString.h"

void UdpConnection::initialize(udp_pcb* pcb /* = nullptr*/)
{
	if (!pcb)
		pcb = udp_new();
	_udp = pcb;
	udp_recv(_udp, staticOnReceive, (void*)this);
}

void UdpConnection::close()
{
	udp_recv(_udp, nullptr, nullptr);
	udp_remove(_udp);
	_udp = nullptr;
}

bool UdpConnection::listen(int port)
{
	if (_udp && _udp->local_port != 0)
		return false;
	else if (!_udp)
		initialize();

	debug_d("UDP listen port %d", port);
	err_t res = udp_bind(_udp, IP_ADDR_ANY, port);
	return res == ERR_OK;
}

bool UdpConnection::connect(IPAddress ip, uint16_t port)
{
	if (!_udp)
		initialize();

	if (_udp->local_port == 0) {
		udp_bind(_udp, IP_ADDR_ANY, 0);
		debug_d("UDP LocalPort: %d", _udp->local_port);
	}

	debug_d("UDP connect to %s:%d", ip.toString().c_str(), port);
	err_t res = udp_connect(_udp, ip, port);
	return res == ERR_OK;
}

void UdpConnection::send(const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	if (p) {
		memcpy(p->payload, data, length);
		udp_send(_udp, p);
		pbuf_free(p);
	}
}

void UdpConnection::sendTo(IPAddress remoteIP, uint16_t remotePort, const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	if (p) {
		memcpy(p->payload, data, length);
		udp_sendto(_udp, p, remoteIP, remotePort);
		pbuf_free(p);
	}
}

void UdpConnection::onReceive(pbuf* buf, IPAddress remoteIP, uint16_t remotePort)
{
	debug_d("UDP received: %d bytes", buf->tot_len);
	if (_onDataCallback) {
		char* data = new char[buf->tot_len + 1];
		pbuf_copy_partial(buf, data, buf->tot_len, 0);
		data[buf->tot_len] = '\0';

		_onDataCallback(*this, data, buf->tot_len, remoteIP, remotePort);

		delete[] data;
	}
}

void UdpConnection::staticOnReceive(void* arg, struct udp_pcb* pcb, struct pbuf* p, LWIP_IP_ADDR_T* addr, u16_t port)
{
	auto conn = (UdpConnection*)arg;
	if (conn) {
		IPAddress reip = addr ? IPAddress(*addr) : IPAddress();
		conn->onReceive(p, reip, port);
	}
	pbuf_free(p);
}
