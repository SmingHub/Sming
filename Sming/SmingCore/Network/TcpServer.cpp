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

TcpServer::TcpServer(TcpClientConnectDelegate onClientHandler, TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler)
: TcpConnection(false)
{
	clientConnectDelegate = onClientHandler;
	clientReceiveDelegate = clientReceiveDataHandler;
	clientCompleteDelegate = clientCompleteHandler;
	timeOut = 40;
	TcpConnection::setTimeOut(USHRT_MAX);

}


TcpServer::TcpServer(TcpClientDataDelegate clientReceiveDataHandler, TcpClientCompleteDelegate clientCompleteHandler)
: TcpConnection(false)
{
	clientReceiveDelegate = clientReceiveDataHandler;
	clientCompleteDelegate = clientCompleteHandler;
	timeOut = 40;
	TcpConnection::setTimeOut(USHRT_MAX);
}

TcpServer::TcpServer(TcpClientDataDelegate clientReceiveDataHandler)
: TcpConnection(false)
{
	clientReceiveDelegate = clientReceiveDataHandler;
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

	TcpConnection* con = new TcpClient(clientTcp,
									   TcpClientDataDelegate(&TcpServer::onClientReceive,this),
									   TcpClientCompleteDelegate(&TcpServer::onClientComplete,this));

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

#ifdef ENABLE_SSL
void TcpServer::setServerKeyCert(SSLKeyCertPair serverKeyCert) {
	clientKeyCert = serverKeyCert;
}
#endif

bool TcpServer::listen(int port, bool useSsl /*= false */)
{
	if (tcp == NULL)
		initialize(tcp_new());

	err_t res = tcp_bind(tcp, IP_ADDR_ANY, port);
	if (res != ERR_OK) return res;

#ifdef ENABLE_SSL
	this->useSsl = useSsl;

	if(useSsl) {

#ifdef SSL_DEBUG
		sslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
#endif

		sslContext = ssl_ctx_new(sslOptions, sslSessionCacheSize);

		if (!(clientKeyCert.keyLength && clientKeyCert.certificateLength)) {
			debugf("SSL: server certificate and key are not provided!");
			return false;
		}

		if (ssl_obj_memory_load(sslContext, SSL_OBJ_RSA_KEY,
								clientKeyCert.key, clientKeyCert.keyLength,
								clientKeyCert.keyPassword) != SSL_OK) {
			debugf("SSL: Unable to load server private key");
			return false;
		}

		if (ssl_obj_memory_load(sslContext, SSL_OBJ_X509_CERT,
			clientKeyCert.certificate,
			clientKeyCert.certificateLength, NULL) != SSL_OK) {
			debugf("SSL: Unable to load server certificate");
			return false;
		}

		// TODO: test: free the certificate data on server destroy...
		freeClientKeyCert = true;
	}
#endif

	tcp = tcp_listen(tcp);
	tcp_accept(tcp, staticAccept);

	//stateTimer.initializeMs(3500, list_mem).start();
	return true;
}

err_t TcpServer::onAccept(tcp_pcb *clientTcp, err_t err)
{
	// Anti DDoS :-)
	if (system_get_free_heap_size() < minHeapSize)
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

#ifdef ENABLE_SSL
	if(useSsl) {
		int clientfd = axl_append(clientTcp);
		if(clientfd == -1) {
			debugf("SSL: Unable to initiate tcp ");
			return ERR_ABRT;
		}

		debugf("SSL: handshake start (%d ms)", millis());
		client->ssl = ssl_server_new(sslContext, clientfd);
		client->useSsl = true;
	}
#endif

	onClient((TcpClient*)client);

	return ERR_OK;
}

void TcpServer::onClient(TcpClient *client)
{
	activeClients++;
	debugf("TcpServer onClient: %s\r\n", client->getRemoteIp().toString().c_str());
	if (clientConnectDelegate)
	{
		clientConnectDelegate(client);
	}
}

void TcpServer::onClientComplete(TcpClient& client, bool succesfull)
{
	activeClients--;
	debugf("TcpSever onComplete: %s\r\n", client.getRemoteIp().toString().c_str());
	if (clientCompleteDelegate)
	{
		clientCompleteDelegate(client,succesfull);
	}
}

bool TcpServer::onClientReceive (TcpClient& client, char *data, int size)
{
	debugf("TcpSever onReceive: %s, %d bytes\r\n", client.getRemoteIp().toString().c_str(), size);
	debugf("Data: %s", data);
	if (clientReceiveDelegate)
	{
		return clientReceiveDelegate(client, data, size);
	}
	return true;
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
