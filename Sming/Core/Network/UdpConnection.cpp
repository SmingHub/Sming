/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * UdpConnection.cpp
 *
 ****/

#include "UdpConnection.h"
#include "WString.h"

bool UdpConnection::initialize(udp_pcb* pcb)
{
	if(pcb == nullptr) {
		pcb = udp_new();
		if(pcb == nullptr) {
			return false;
		}
	}

	udp = pcb;
	udp_recv(udp, staticOnReceive, this);

	return true;
}

void UdpConnection::close()
{
	udp_recv(udp, nullptr, nullptr);
	udp_remove(udp);
	udp = nullptr;
}

bool UdpConnection::listen(int port)
{
	if(udp == nullptr) {
		if(!initialize()) {
			return false;
		}
	} else if(udp->local_port != 0) {
		return false;
	}

	debug_d("UDP listen port %d", port);
	err_t res = udp_bind(udp, IP_ADDR_ANY, port);
	return res == ERR_OK;
}

bool UdpConnection::connect(IpAddress ip, uint16_t port)
{
	if(udp == nullptr) {
		if(!initialize()) {
			return false;
		}
	}

	if(udp->local_port == 0) {
		udp_bind(udp, IP_ADDR_ANY, 0);
		debug_d("UDP LocalPort: %d", udp->local_port);
	}

	debug_d("UDP connect to %s:%d", ip.toString().c_str(), port);
	err_t res = udp_connect(udp, ip, port);
	return res == ERR_OK;
}

bool UdpConnection::send(const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	if(p == nullptr) {
		return false;
	} else {
		memcpy(p->payload, data, length);
		err_t res = udp_send(udp, p);
		pbuf_free(p);
		return res == ERR_OK;
	}
}

bool UdpConnection::sendTo(IpAddress remoteIP, uint16_t remotePort, const char* data, int length)
{
	pbuf* p = pbuf_alloc(PBUF_TRANSPORT, length, PBUF_RAM);
	if(p == nullptr) {
		return false;
	} else {
		memcpy(p->payload, data, length);
		err_t res = udp_sendto(udp, p, remoteIP, remotePort);
		pbuf_free(p);
		return res == ERR_OK;
	}
}

void UdpConnection::onReceive(pbuf* buf, IpAddress remoteIP, uint16_t remotePort)
{
	debug_d("UDP received: %d bytes", buf->tot_len);
	if(onDataCallback) {
		auto data = new char[buf->tot_len + 1];
		pbuf_copy_partial(buf, data, buf->tot_len, 0);
		data[buf->tot_len] = '\0';

		onDataCallback(*this, data, buf->tot_len, remoteIP, remotePort);

		delete[] data;
	}
}

void UdpConnection::staticOnReceive(void* arg, struct udp_pcb* pcb, struct pbuf* p, LWIP_IP_ADDR_T* addr, u16_t port)
{
	auto conn = static_cast<UdpConnection*>(arg);
	if(conn != nullptr) {
		IpAddress reip = addr != nullptr ? IpAddress(*addr) : IpAddress();
		conn->onReceive(p, reip, port);
	}
	pbuf_free(p);
}
