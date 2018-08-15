/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpServer.h"

TcpConnection* TcpServer::createClient(tcp_pcb* clientTcp)
{
	debug_d("TCP Server createClient %snull\r\n", clientTcp ? "not" : "");

	if (!_active) {
		debug_w("Refusing new connections. The server is shutting down");
		return nullptr;
	}

	TcpConnection* con = new TcpClient(clientTcp, TcpClientDataDelegate(&TcpServer::onClientReceive, this),
									   TcpClientCompleteDelegate(&TcpServer::onClientComplete, this));

	return con;
}

void list_mem()
{
	debug_d("Free heap size=%d", system_get_free_heap_size());
}

void TcpServer::setTimeOut(uint16_t waitTimeOut)
{
	debug_d("Server timeout updating: %d -> %d", _timeOut, waitTimeOut);
	_timeOut = waitTimeOut;
}

bool TcpServer::listen(int port, bool useSsl /*= false */)
{
	if (!_tcp)
		initialize(tcp_new());

	err_t res = tcp_bind(_tcp, IP_ADDR_ANY, port);
	if (res != ERR_OK)
		return res;

#ifdef ENABLE_SSL
	_useSsl = useSsl;

	if (useSsl) {
#ifdef SSL_DEBUG
		_sslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
#endif

		_sslContext = ssl_ctx_new(_sslOptions, _sslSessionCacheSize);

		if (!(_sslKeyCert.keyLength && _sslKeyCert.certificateLength)) {
			debug_e("SSL: server certificate and key are not provided!");
			return false;
		}

		if (ssl_obj_memory_load(_sslContext, SSL_OBJ_RSA_KEY, _sslKeyCert.key, _sslKeyCert.keyLength,
								_sslKeyCert.keyPassword) != SSL_OK) {
			debug_e("SSL: Unable to load server private key");
			return false;
		}

		if (ssl_obj_memory_load(_sslContext, SSL_OBJ_X509_CERT, _sslKeyCert.certificate, _sslKeyCert.certificateLength,
								nullptr) != SSL_OK) {
			debug_e("SSL: Unable to load server certificate");
			return false;
		}

		// TODO: test: free the certificate data on server destroy...
		_freeKeyCert = true;
	}
#endif

	_tcp = tcp_listen(_tcp);
	tcp_accept(_tcp, staticAccept);

	return true;
}

err_t TcpServer::onAccept(tcp_pcb* clientTcp, err_t err)
{
	// Anti DDoS :-)
	if (system_get_free_heap_size() < _minHeapSize) {
		debug_w("\r\n\r\nCONNECTION DROPPED\r\n\t(%d)\r\n\r\n", system_get_free_heap_size());
		return ERR_MEM;
	}

#ifdef NETWORK_DEBUG
	debug_d("onAccept state: %d K=%d", err, _connections.count());
	list_mem();
#endif

	if (err != ERR_OK) {
		//closeTcpConnection(clientTcp, nullptr);
		return err;
	}

	TcpConnection* client = createClient(clientTcp);
	if (!client)
		return ERR_MEM;
	client->setTimeOut(_timeOut);

#ifdef ENABLE_SSL
	if (_useSsl) {
		int clientfd = axl_append(clientTcp);
		if (clientfd == -1) {
			delete client;
			debug_e("SSL: Unable to initiate tcp ");
			return ERR_ABRT;
		}

		debug_d("SSL: handshake start (%d ms)", millis());
		client->setSsl(ssl_server_new(_sslContext, clientfd));
	}
#endif

	client->setDestroyedDelegate(TcpConnectionDestroyedDelegate(&TcpServer::onClientDestroy, this));

	_connections.add(client);
	debug_d("Opening connection. Total connections: %d", _connections.count());

	onClient((TcpClient*)client);

	return ERR_OK;
}

void TcpServer::onClient(TcpClient* client)
{
	debug_d("TcpServer onClient: %s\r\n", client->getRemoteIp().toString().c_str());
	if (_clientConnectDelegate) {
		_clientConnectDelegate(client);
	}
}

void TcpServer::onClientComplete(TcpClient& client, bool successful)
{
	debug_d("TcpSever onComplete: %s\r\n", client.getRemoteIp().toString().c_str());
	if (_clientCompleteDelegate)
		_clientCompleteDelegate(client, successful);
}

bool TcpServer::onClientReceive(TcpClient& client, char* data, int size)
{
	debug_d("TcpSever onReceive: %s, %d bytes\r\n", client.getRemoteIp().toString().c_str(), size);
	debug_d("Data: %s", data);
	if (_clientReceiveDelegate) {
		return _clientReceiveDelegate(client, data, size);
	}
	return true;
}

err_t TcpServer::staticAccept(void* arg, tcp_pcb* new_tcp, err_t err)
{
	TcpServer* con = (TcpServer*)arg;

	if (!con) {
		debug_e("NO CONNECTION ON TCP");
		//closeTcpConnection(new_tcp);
		tcp_abort(new_tcp);
		return ERR_ABRT;
	}

	con->_sleep = 0;

	err_t res = con->onAccept(new_tcp, err);
	return res;
}

void TcpServer::shutdown()
{
	_active = false;

	debug_i("Shutting down the server ...");

	if (_tcp) {
		tcp_arg(_tcp, nullptr);
		tcp_accept(_tcp, nullptr);
		tcp_close(_tcp);

		_tcp = nullptr;
	}

	if (!_connections.count()) {
		delete this;
		return;
	}

	for (unsigned i = 0; i < _connections.count(); i++) {
		auto connection = _connections[i];
		if (connection)
			connection->setTimeOut(1);
	}
}

void TcpServer::onClientDestroy(TcpConnection& connection)
{
	_connections.removeElement((TcpConnection*)&connection);
	debug_d("Destroying connection. Total connections: %d", _connections.count());

	if (_active) {
		return;
	}

	if (_connections.count() == 0) {
		delete this;
	}
}
