/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpServer.h
 *
 ****/

/** @defgroup tcpserver Servers
 *  @brief Provides the base for building TCP servers
 *  @ingroup tcp
 *
 *  @{
 */

#pragma once

#include "TcpConnection.h"
#include "TcpClient.h"

using TcpClientConnectDelegate = Delegate<void(TcpClient* client)>;

// By default a TCP server will wait for a new remote client connection to get established for 20 seconds
#ifndef TCP_SERVER_TIMEOUT
#define TCP_SERVER_TIMEOUT 20
#endif

class TcpServer : public TcpConnection
{
public:
	TcpServer() : TcpConnection(false)
	{
		TcpConnection::timeOut = TCP_SERVER_TIMEOUT;
	}

	TcpServer(TcpClientConnectDelegate onClientHandler, TcpClientDataDelegate clientReceiveDataHandler,
			  TcpClientCompleteDelegate clientCompleteHandler)
		: TcpConnection(false), clientConnectDelegate(onClientHandler), clientReceiveDelegate(clientReceiveDataHandler),
		  clientCompleteDelegate(clientCompleteHandler)
	{
		TcpConnection::timeOut = TCP_SERVER_TIMEOUT;
	}

	TcpServer(TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler)
		: TcpConnection(false), clientReceiveDelegate(clientReceiveDataHandler),
		  clientCompleteDelegate(clientCompleteHandler)
	{
		TcpConnection::timeOut = TCP_SERVER_TIMEOUT;
	}

	TcpServer(TcpClientDataDelegate clientReceiveDataHandler)
		: TcpConnection(false), clientReceiveDelegate(clientReceiveDataHandler)
	{
		TcpConnection::timeOut = TCP_SERVER_TIMEOUT;
	}

	~TcpServer()
	{
		debug_i("TcpServer destroyed");
	}

	virtual bool listen(int port, bool useSsl = false);

	void setKeepAlive(uint16_t seconds);

	void shutdown();

	const Vector<TcpConnection*>& getConnections() const
	{
		return connections;
	}

protected:
	// Overload this method in your derived class!
	virtual TcpConnection* createClient(tcp_pcb* clientTcp);

	virtual err_t onAccept(tcp_pcb* clientTcp, err_t err);
	virtual void onClient(TcpClient* client);
	virtual bool onClientReceive(TcpClient& client, char* data, int size);
	virtual void onClientComplete(TcpClient& client, bool successful);
	virtual void onClientDestroy(TcpConnection& connection);

private:
	static err_t staticAccept(void* arg, tcp_pcb* new_tcp, err_t err);

public:
	uint16_t activeClients = 0;

protected:
	size_t minHeapSize = 16384;
	uint16_t maxConnections = 0; ///< By default, don't limit connection count

	bool active = true;
	Vector<TcpConnection*> connections;

private:
	uint16_t keepAlive = 70; ///< The time to wait after the connection is established. If there is no data
							 //  coming or going to the client within that period the client connection will be closed
	TcpClientConnectDelegate clientConnectDelegate = nullptr;
	TcpClientDataDelegate clientReceiveDelegate = nullptr;
	TcpClientCompleteDelegate clientCompleteDelegate = nullptr;
};

/** @} */
