/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_TCPSERVER_H_
#define _SMING_CORE_TCPSERVER_H_

#include "TcpConnection.h"
#include "TcpClient.h"

class TcpServer: public TcpConnection {
public:
	TcpServer();
	TcpServer(TcpClientDataDelegate clientReceiveDataHandler);
	virtual ~TcpServer();

public:
	virtual bool listen(int port);
	void setTimeOut(uint16_t waitTimeOut);

protected:
	// Overload this method in your derived class!
	virtual TcpConnection* createClient(tcp_pcb *clientTcp);

	virtual err_t onAccept(tcp_pcb *clientTcp, err_t err);
	virtual void onClient(TcpConnection *client);

	static err_t staticAccept(void *arg, tcp_pcb *new_tcp, err_t err);

public:
	static int16_t totalConnections;

private:
	uint16_t timeOut;
	TcpClientDataDelegate clientReceive;
};

#endif /* _SMING_CORE_TCPSERVER_H_ */
