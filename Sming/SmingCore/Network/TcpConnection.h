/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_TCPCONNECTION_H_
#define _SMING_CORE_TCPCONNECTION_H_

#include "../Wiring/WiringFrameworkDependencies.h"
#include "IPAddress.h"

#define NETWORK_DEBUG

#define NETWORK_SEND_BUFFER_SIZE 1024

enum TcpConnectionEvent
{
	// Occurs after connection establishment
	eTCE_Connected = 0,
	// Occurs on data receive
	eTCE_Received,
	// Occurs when previous sending was completed
	eTCE_Sent,
	// Occurs on waiting
	eTCE_Poll
};

struct pbuf;
class String;
class IDataSourceStream;
class IPAddress;

class TcpConnection
{
public:
	TcpConnection(bool autoDestruct);
	TcpConnection(tcp_pcb* connection, bool autoDestruct);
	virtual ~TcpConnection();

public:
	virtual bool connect(String server, int port);
	virtual bool connect(IPAddress addr, uint16_t port);
	virtual void close();

	// return -1 on error
	int writeString(const char* data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	int writeString(const String data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	// return -1 on error
	virtual int write(const char* data, int len, uint8_t apiflags = TCP_WRITE_FLAG_COPY); // flags: TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE
	int write(IDataSourceStream* stream);
	__forceinline uint16_t getAvailableWriteSize() { return (canSend && tcp) ? tcp_sndbuf(tcp) : 0; }
	void flush();

	void setTimeOut(uint16_t waitTimeOut);
	IPAddress getRemoteIp()  { return (tcp == NULL) ? INADDR_NONE : IPAddress(tcp->remote_ip);};
	uint16_t getRemotePort() { return (tcp == NULL) ? 0 : tcp->remote_port; };

protected:
	bool internalTcpConnect(IPAddress addr, uint16_t port);
	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf *buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

	static err_t staticOnConnected(void *arg, tcp_pcb *tcp, err_t err);
	static err_t staticOnReceive(void *arg, tcp_pcb *tcp, pbuf *p, err_t err);
	static err_t staticOnSent(void *arg, tcp_pcb *tcp, uint16_t len);
	static err_t staticOnPoll(void *arg, tcp_pcb *tcp);
	static void staticOnError(void *arg, err_t err);
	static void staticDnsResponse(const char *name, ip_addr_t *ipaddr, void *arg);

	static void closeTcpConnection(tcp_pcb *tpcb);
	void initialize(tcp_pcb* pcb);

private:
	inline void checkSelfFree() { if (tcp == NULL && autoSelfDestruct) delete this; }

protected:
	tcp_pcb *tcp;
	uint16_t sleep;
	uint16_t timeOut;
	bool canSend;
	bool autoSelfDestruct;
};

#endif /* _SMING_CORE_TCPCONNECTION_H_ */
