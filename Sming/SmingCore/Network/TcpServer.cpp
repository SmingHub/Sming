/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpServer.h"
#include "TcpClient.h"

#include "../../SmingCore/Digital.h"
#include "../../SmingCore/Timer.h"

int16_t TcpServer::totalConnections = 0;

TcpServer::TcpServer() : TcpConnection(false)
{
	timeOut = 40;
	TcpConnection::setTimeOut(USHRT_MAX);
}

TcpServer::TcpServer(TcpClientDataDelegate clientReceiveDataHandler) : TcpConnection(false)
{
	clientReceive = clientReceiveDataHandler;
	timeOut = 40;
	TcpConnection::setTimeOut(USHRT_MAX);
}

TcpServer::~TcpServer()
{
}

TcpConnection* TcpServer::createClient(tcp_pcb *clientTcp)
{
	if (clientTcp == NULL)
	{
		debugf("TCP Server createClient NULL\r\n");
	}
	else
	{
		debugf("TCP Server createClient not NULL");
	}

	TcpConnection* con = new TcpClient(clientTcp, clientReceive, true);
	return con;
}

//Timer stateTimer;
void list_mem()
{
	debugf("Free heap size=%d, K=%d", system_get_free_heap_size(), TcpServer::totalConnections);
}

void TcpServer::setTimeOut(uint16_t waitTimeOut)
{
	debugf("Server timeout updating: %d -> %d", timeOut, waitTimeOut);
	timeOut = waitTimeOut;
}

bool TcpServer::listen(int port)
{
	if (tcp == NULL)
		tcp = tcp_new();

	err_t res = tcp_bind(tcp, IP_ADDR_ANY, port);
	if (res != ERR_OK) return res;

	tcp = tcp_listen(tcp);
	tcp_accept(tcp, staticAccept);

	//stateTimer.initializeMs(3500, list_mem).start();
	return true;
}

err_t TcpServer::onAccept(tcp_pcb *clientTcp, err_t err)
{
	// Anti DDoS :-)
	if (system_get_free_heap_size() < 6500)
	{
		debugf("\r\n\r\nCONNECTION DROPPED\r\n\t(%d)\r\n\r\n", system_get_free_heap_size());
		return ERR_MEM;
	}

	#ifdef NETWORK_DEBUG
	debugf("onAccept state: %d K=%d", err, totalConnections);
	list_mem();
	#endif

	if (err != ERR_OK)
	{
		//closeTcpConnection(clientTcp, NULL);
		return err;
	}

	TcpConnection* client = createClient(clientTcp);
	if (client == NULL) return ERR_MEM;
	client->setTimeOut(timeOut);
	onClient(client);

	return ERR_OK;
}

void TcpServer::onClient(TcpConnection *connection)
{
	debugf("Tcp Server onClient ") ; // %s",connection->getRemoteIp().toString().c_str());
}

err_t TcpServer::staticAccept(void *arg, tcp_pcb *new_tcp, err_t err)
{
	TcpServer* con = (TcpServer*)arg;

	if (con == NULL)
	{
		debugf("NO CONNECTION ON TCP");
		//closeTcpConnection(new_tcp);
		tcp_abort(new_tcp);
		return ERR_ABRT;
	}
	else
		con->sleep = 0;

	err_t res = con->onAccept(new_tcp, err);
	return res;
}
