/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup tcpserver Servers
 *  @brief Provides the base for building TCP servers
 *  @ingroup tcp
 *
 *  @{
 */

#ifndef _SMING_CORE_TCPSERVER_H_
#define _SMING_CORE_TCPSERVER_H_

#include "TcpConnection.h"
#include "TcpClient.h"

typedef Delegate<void(TcpClient* client)> TcpClientConnectDelegate;

class TcpServer : public TcpConnection {
public:
	TcpServer() : TcpServer(nullptr, nullptr, nullptr)
	{}

	TcpServer(TcpClientDataDelegate clientReceiveDataHandler) : TcpServer(nullptr, clientReceiveDataHandler, nullptr)
	{}

	TcpServer(TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler) :
		TcpServer(nullptr, clientReceiveDataHandler, clientCompleteHandler)
	{}

	TcpServer(TcpClientConnectDelegate onClientHandler, TcpClientDataDelegate clientReceiveDataHandler,
			  TcpClientCompleteDelegate clientCompleteHandler) :
		TcpConnection(false),
		_clientConnectDelegate(onClientHandler),
		_clientReceiveDelegate(clientReceiveDataHandler),
		_clientCompleteDelegate(clientCompleteHandler)
	{
		TcpConnection::setTimeOut(USHRT_MAX);
	}

	virtual ~TcpServer()
	{
		debug_i("Server is destroyed.");
	}

	virtual bool listen(int port, bool useSsl = false);
	void setTimeOut(uint16_t waitTimeOut);

	void shutdown();

#ifdef ENABLE_SSL
	/**
	 * @brief Adds SSL support and specifies the server certificate and private key.
	 * @deprecated: Use setSslKeyCert instead
	 */
	void setServerKeyCert(SSLKeyCertPair serverKeyCert)
	{
		setSslKeyCert(serverKeyCert);
	}

	/**
	 * @brief Adds SSL support and specifies the server certificate and private key.
	 */
	using TcpConnection::setSslKeyCert;
#endif

	const Vector<TcpConnection*>& connections()
	{
		return _connections;
	}

protected:
	// Overload this method in your derived class!
	virtual TcpConnection* createClient(tcp_pcb* clientTcp);

	virtual err_t onAccept(tcp_pcb* clientTcp, err_t err);
	virtual void onClient(TcpClient* client);
	virtual bool onClientReceive(TcpClient& client, char* data, int size);
	virtual void onClientComplete(TcpClient& client, bool successful);
	virtual void onClientDestroy(TcpConnection& connection);

	static err_t staticAccept(void* arg, tcp_pcb* new_tcp, err_t err);

protected:
	size_t _minHeapSize = 3000;

#ifdef ENABLE_SSL
	int _sslSessionCacheSize = 50;
#endif

	bool _active = true;
	Vector<TcpConnection*> _connections;

private:
	uint16_t _timeOut = 40;
	TcpClientConnectDelegate _clientConnectDelegate = nullptr;
	TcpClientDataDelegate _clientReceiveDelegate = nullptr;
	TcpClientCompleteDelegate _clientCompleteDelegate = nullptr;
};

/** @} */
#endif /* _SMING_CORE_TCPSERVER_H_ */
