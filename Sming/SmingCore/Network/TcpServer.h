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

typedef Delegate<void(TcpClient* client)> TcpClientConnectDelegate;

class TcpServer: public TcpConnection {
public:
	TcpServer();
	TcpServer(TcpClientConnectDelegate onClientHandler, TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler);
	TcpServer(TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler);
	TcpServer(TcpClientDataDelegate clientReceiveDataHandler);
	virtual ~TcpServer();

public:
	virtual bool listen(int port, bool useSsl = false);
	void setTimeOut(uint16_t waitTimeOut);

#ifdef ENABLE_SSL
	/**
	 * @brief Adds SSL support and specifies the server certificate and private key.
	 */
	void setServerKeyCert(SSLKeyCertPair serverKeyCert);
#endif

protected:
	// Overload this method in your derived class!
	virtual TcpConnection* createClient(tcp_pcb *clientTcp);

	virtual err_t onAccept(tcp_pcb *clientTcp, err_t err);
	virtual void onClient(TcpClient *client);
	virtual void onClientComplete(TcpClient& client, bool succesfull);
	virtual bool onClientReceive (TcpClient& client, char *data, int size);

	static err_t staticAccept(void *arg, tcp_pcb *new_tcp, err_t err);

public:
	static int16_t totalConnections;
	uint16_t activeClients = 0;

protected:
	int minHeapSize = 6500;

#ifdef ENABLE_SSL
	int sslSessionCacheSize = 50;
#endif

private:
	uint16_t timeOut;
	TcpClientDataDelegate clientReceiveDelegate = NULL;
	TcpClientCompleteDelegate clientCompleteDelegate = NULL;
	TcpClientConnectDelegate clientConnectDelegate = NULL;
};

#endif /* _SMING_CORE_TCPSERVER_H_ */
