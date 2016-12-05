/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpConnection.h"

#include "../../SmingCore/DataSourceStream.h"
#include "../../SmingCore/Platform/WDT.h"
#include "NetUtils.h"
#include "../Wiring/WString.h"
#include "../Wiring/IPAddress.h"

#ifdef ENABLE_SSL
#include "../Clock.h"
#endif

TcpConnection::TcpConnection(bool autoDestruct) : autoSelfDestruct(autoDestruct), sleep(0), canSend(true), timeOut(70)
{
	initialize(tcp_new());
}

TcpConnection::TcpConnection(tcp_pcb* connection, bool autoDestruct) : autoSelfDestruct(autoDestruct), sleep(0), canSend(true), timeOut(70)
{
	initialize(connection);
}

TcpConnection::~TcpConnection()
{
	autoSelfDestruct = false;
	close();

	if(sslFingerprint) {
		delete[] sslFingerprint;
	}
	freeSslClientKeyCert();
	debugf("~TCP connection");
}

bool TcpConnection::connect(String server, int port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (tcp == NULL)
		initialize(tcp_new());

	ip_addr_t addr;

	this->useSsl = useSsl;
	this->sslOptions |= sslOptions;

#ifdef ENABLE_SSL
	if(ssl_ext == NULL) {
		ssl_ext = ssl_ext_new();
		ssl_ext->host_name = (char *)malloc(server.length() + 1);
		strcpy(ssl_ext->host_name, server.c_str());

		ssl_ext->max_fragment_size = 4*1024; // 4K max size
	}
#endif

	debugf("connect to: %s", server.c_str());
	canSend = false; // Wait for connection
	DnsLookup *look = new DnsLookup { this, port };
	err_t dnslook = dns_gethostbyname(server.c_str(), &addr, staticDnsResponse, look);
	if (dnslook != ERR_OK)
	{
		if (dnslook == ERR_INPROGRESS)
			return true;
		else
		{
			delete look;
			return false;
		}
	}
	delete look;

	return internalTcpConnect(addr, port);
}

bool TcpConnection::connect(IPAddress addr, uint16_t port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{

	this->useSsl = useSsl;
	this->sslOptions |= sslOptions;

	return internalTcpConnect(addr, port);
}

void TcpConnection::setTimeOut(uint16_t waitTimeOut)
{
	debugf("timeout updating: %d -> %d", timeOut, waitTimeOut);
	timeOut = waitTimeOut;
}

err_t TcpConnection::onReceive(pbuf *buf)
{
	if (buf == NULL)
		debugf("TCP received: (null)");
	else
		debugf("TCP received: %d bytes", buf->tot_len);

	if (buf != NULL && getAvailableWriteSize() > 0)
		onReadyToSendData(eTCE_Received);

	return ERR_OK;
}

err_t TcpConnection::onSent(uint16_t len)
{
	debugf("TCP sent: %d", len);

	//debugf("%d %d", tcp->state, tcp->flags); // WRONG!
	if (len >= 0 && tcp != NULL && getAvailableWriteSize() > 0)
		onReadyToSendData(eTCE_Sent);

	return ERR_OK;
}

err_t TcpConnection::onPoll()
{
	if (sleep >= timeOut && timeOut != USHRT_MAX)
	{
		debugf("TCP connection closed by timeout: %d (from %d)", sleep, timeOut);

		close();
		return ERR_TIMEOUT;
	}

	if (tcp != NULL && getAvailableWriteSize() > 0) //(tcp->state >= SYN_SENT && tcp->state <= ESTABLISHED))
		onReadyToSendData(eTCE_Poll);

	return ERR_OK;
}

err_t TcpConnection::onConnected(err_t err)
{
	if (err != ERR_OK)
		debugf("TCP connected error status: %d", err);
	else
		debugf("TCP connected");

	canSend = true;
	if (err == ERR_OK)
		onReadyToSendData(eTCE_Connected);
	else
		close();

	return ERR_OK;
}

void TcpConnection::onError(err_t err)
{
#ifdef ENABLE_SSL
	if(ssl) {
//		ssl_ctx_free(sslContext);
		ssl_free(ssl);
		sslContext=nullptr;
		ssl=nullptr;
		sslConnected = false;
	}
#endif
	debugf("TCP connection error: %d", err);
}

void TcpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if (sourceEvent != eTCE_Poll) debugf("onReadyToSendData: %d", sourceEvent);
}

int TcpConnection::writeString(const String data, uint8_t apiflags /* = TCP_WRITE_FLAG_COPY*/)
{
	writeString(data.c_str(), apiflags);
}

int TcpConnection::writeString(const char* data, uint8_t apiflags /* = TCP_WRITE_FLAG_COPY*/)
{
	return write(data, strlen(data), apiflags);
}

int TcpConnection::write(const char* data, int len, uint8_t apiflags /* = TCP_WRITE_FLAG_COPY*/)
{
   WDT.alive();

   err_t err = ERR_OK;

#ifdef ENABLE_SSL
   if(ssl) {
		int written = axl_ssl_write(ssl, (const uint8_t *)data, len);
		// debugf("SSL: Write len: %d, Written: %d", len, written);
		if(written < ERR_OK) {
			err = written;
			debugf("SSL: Write Error: %d", err);
		}
   }
   else {
#endif
	   u16_t available = getAvailableWriteSize();
	   if (available < len) {
		   if (available == 0)
			   return -1; // No memory
		   else
			   len = available;
	   }
	   err = tcp_write(tcp, data, len, apiflags);

#ifdef ENABLE_SSL
   }
#endif

   if (err == ERR_OK)
   {
		//debugf("TCP connection send: %d (%d)", len, original);
		return len;
   } else {
		//debugf("TCP connection failed with err %d (\"%s\")", err, lwip_strerr(err));
		return -1;
   }
}

int TcpConnection::write(IDataSourceStream* stream)
{
#ifdef ENABLE_SSL
	if(ssl && !sslConnected) {
		// wait until the SSL handshake is done.
		return 0;
	}
#endif

	// Send data from DataStream
	bool repeat;
	bool space;
	int available;
	int total = 0;
	char buffer[NETWORK_SEND_BUFFER_SIZE];

	do
	{
		space = (tcp_sndqueuelen(tcp) < TCP_SND_QUEUELEN);
		if (!space)
		{
			debugf("WAIT FOR FREE SPACE");
			flush();
			break; // don't try to send buffers if no free space available
		}

		// Join small fragments
		int pushCount = 0;
		do
		{
			pushCount++;
			int read = min(NETWORK_SEND_BUFFER_SIZE, getAvailableWriteSize());
			if (read > 0)
				available = stream->readMemoryBlock(buffer, read);
			else
				available = 0;

			if (available > 0)
			{
				int written = write(buffer, available, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
				total += written;
				stream->seek(max(written, 0));
				debugf("Written: %d, Available: %d, isFinished: %d, PushCount: %d", written, available, (stream->isFinished()?1:0), pushCount);
				repeat = written == available && !stream->isFinished() && pushCount < 25;
			}
			else
				repeat = false;
		} while (repeat);

		space = (tcp_sndqueuelen(tcp) < TCP_SND_QUEUELEN);// && tcp_sndbuf(tcp) >= FILE_STREAM_BUFFER_SIZE;
	} while (repeat && space);

	flush();
	return total;
}

void TcpConnection::close()
{
#ifdef ENABLE_SSL
	if (ssl != nullptr) {
		debugf("SSL: closing ...");
//		ssl_ctx_free(sslContext);
		ssl_free(ssl);
		sslContext=nullptr;
		ssl=nullptr;
		sslConnected = false;
		debugf("done\n");
	}
#endif

	if (tcp == NULL) return;
	debugf("TCP connection closing");

#ifdef ENABLE_SSL
	axl_free(tcp);
#endif

	tcp_poll(tcp, staticOnPoll, 1);
	tcp_arg(tcp, NULL); // reset pointer to close connection on next callback
	tcp = NULL;

	checkSelfFree();
}

void TcpConnection::initialize(tcp_pcb* pcb)
{
	tcp = pcb;
	sleep = 0;
	canSend = true;
#ifdef ENABLE_SSL
	axl_init(10);
#endif

	tcp_nagle_disable(tcp);
	tcp_arg(tcp, (void*)this);
	tcp_sent(tcp, staticOnSent);
	tcp_recv(tcp, staticOnReceive);
	tcp_err(tcp, staticOnError);
	tcp_poll(tcp, staticOnPoll, 4);

	#ifdef NETWORK_DEBUG
	debugf("+TCP connection");
	#endif
}

void TcpConnection::closeTcpConnection(tcp_pcb *tpcb)
{
	if (tpcb == NULL) return;

	debugf("-TCP connection");

	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);
	tcp_accept(tpcb, NULL);

	auto err = tcp_close(tpcb);
	if (err != ERR_OK)
	{
		debugf("tcp wait close connection");
		/* error closing, try again later in poll */
		tcp_poll(tpcb, staticOnPoll, 4);
	}
}

void TcpConnection::flush()
{
	if (tcp->state == ESTABLISHED)
	{
		//debugf("TCP flush()");
		tcp_output(tcp);
	}
}

bool TcpConnection::internalTcpConnect(IPAddress addr, uint16_t port)
{
	NetUtils::FixNetworkRouting();
	err_t res = tcp_connect(tcp, addr, port, staticOnConnected);
	debugf("TcpConnection::connect result:, %d", res);
	return res == ERR_OK;
}

err_t TcpConnection::staticOnConnected(void *arg, tcp_pcb *tcp, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	if (con == NULL)
	{
		debugf("OnConnected ABORT");
		//closeTcpConnection(tcp);
		tcp_abort(tcp);
		return ERR_ABRT;
	}
	else
		debugf("OnConnected");

#ifndef ENABLE_SSL
	if(con->useSsl) {
		debugf("WARNING: SSL is not compiled. Make sure to compile Sming with 'make ENABLE_SSL=1' ");
	}
#else
	debugf("staticOnConnected: useSSL: %d, Error: %d", con->useSsl, err);

	if(con->useSsl && err == ERR_OK) {
		int clientfd = axl_append(tcp);
		if(clientfd == -1) {
			debugf("SSL: Unable to add LWIP tcp -> clientfd mapping");
				return ERR_OK;
		}
		else {
			uint32_t sslOptions = con->sslOptions;
#ifdef SSL_DEBUG
			sslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
			debugf("SSL: Show debug data ...");
#endif
			debugf("SSL: Starting connection...");
#ifndef SSL_SLOW_CONNECT
			debugf("SSL: Switching to 160 MHz");
			System.setCpuFrequency(eCF_160MHz); // For shorter waiting time, more power consumption.
#endif
			debugf("SSL: handshake start (%d ms)", millis());
			con->sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | sslOptions, 1);

			if (con->clientKeyCert.keyLength && con->clientKeyCert.certificateLength) {
				// if we have client certificate -> try to use it.
				if (ssl_obj_memory_load(con->sslContext, SSL_OBJ_RSA_KEY,
						con->clientKeyCert.key, con->clientKeyCert.keyLength,
						con->clientKeyCert.keyPassword) != SSL_OK) {
					debugf("SSL: Unable to load client private key");
				} else if (ssl_obj_memory_load(con->sslContext, SSL_OBJ_X509_CERT,
						con->clientKeyCert.certificate,
						con->clientKeyCert.certificateLength, NULL) != SSL_OK) {
					debugf("SSL: Unable to load client certificate");
				}

				if(con->freeClientKeyCert) {
					con->freeSslClientKeyCert();
				}
			}

			con->ssl = ssl_client_new(con->sslContext, clientfd, NULL, 0, con->ssl_ext);
			if(ssl_handshake_status(con->ssl)!=SSL_OK) {
				debugf("SSL: handshake is in progress...");
				return SSL_OK;
			}

#ifndef SSL_SLOW_CONNECT
			debugf("SSL: Switching back 80 MHz");
			System.setCpuFrequency(eCF_80MHz);
#endif
		}
	}
#endif

	err_t res = con->onConnected(err);
	con->checkSelfFree();
	//debugf("<staticOnConnected");
	return res;
}

err_t TcpConnection::staticOnReceive(void *arg, tcp_pcb *tcp, pbuf *p, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	err_t ret_err;
	//Serial.println("echo_recv!");

	if (con == NULL)
	{
		if (p != NULL)
		{
		  /* Inform TCP that we have taken the data. */
		  tcp_recved(tcp, p->tot_len);
		  pbuf_free(p);
		}
		closeTcpConnection(tcp);
		return ERR_OK;
	}
	else
		con->sleep = 0;

	if (err != ERR_OK /*&& err != ERR_CLSD && err != ERR_RST*/)
	{
		debugf("Received ERROR %d", err);
		/* exit and free resources, for unknown reason */
		if (p != NULL)
		{
		  /* Inform TCP that we have taken the data. */
		  tcp_recved(tcp, p->tot_len);
		  pbuf_free(p);
		}
		closeTcpConnection(tcp); // ??
		con->tcp = NULL;
		con->onError(err);
		//con->close();
		return err == ERR_ABRT ? ERR_ABRT : ERR_OK;
	}

	//if (tcp != NULL && tcp->state == ESTABLISHED) // If active
	/* We have taken the data. */
	if (p != NULL) {
		tcp_recved(tcp, p->tot_len);
	}
	else {
		debugf("TcpConnection::staticOnReceive: pbuf is NULL");
	}

#ifdef ENABLE_SSL
	if(con->ssl && p != NULL) {
		WDT.alive(); /* SSL handshake needs time. In theory we have max 8 seconds before the hardware watchdog resets the device */
		struct pbuf* pout;

		int read_bytes = axl_ssl_read(con->ssl, tcp, p, &pout);

		// free the SSL pbuf and put the decrypted data in the brand new pout pbuf
		if(p != NULL) {
			pbuf_free(p);
		}

		if(read_bytes < SSL_OK) {
			debugf("SSL: Got error: %d", read_bytes);
			if(read_bytes == SSL_CLOSE_NOTIFY) {
				return ERR_OK;
			}

			con->close();
			closeTcpConnection(tcp);
			return read_bytes;
		}

		if (read_bytes == 0) {
			if(!con->sslConnected && ssl_handshake_status(con->ssl) == SSL_OK) {
				con->sslConnected = true;
				debugf("SSL: Handshake done (%d ms).", millis());
#ifndef SSL_SLOW_CONNECT
				debugf("SSL: Switching back to 80 MHz");
				System.setCpuFrequency(eCF_80MHz); // Preserve some CPU cycles
#endif
				if(con->sslFingerprint && ssl_match_fingerprint(con->ssl, con->sslFingerprint) != SSL_OK) {
					debugf("SSL: Certificate fingerprint does not match!");
					con->close();
					closeTcpConnection(tcp);

					return ERR_ABRT;
				}

				err_t res = con->onConnected(err);
				con->checkSelfFree();

				return res;
			}

			// No data yet
			return ERR_OK;
		}

		// we got some decrypted bytes...
		debugf("SSL: Decrypted data len %d", read_bytes);

		// put the decrypted data in a brand new pbuf
		p = pout;
	}
#endif

	err_t res = con->onReceive(p);

	if (p != NULL)
		pbuf_free(p);
	else
	{
		con->close();
		closeTcpConnection(tcp);
	}

	con->checkSelfFree();
	//debugf("<staticOnReceive");
	return res;
}

err_t TcpConnection::staticOnSent(void *arg, tcp_pcb *tcp, uint16_t len)
{
	TcpConnection* con = (TcpConnection*)arg;

	if (con == NULL)
		return ERR_OK;
	else
		con->sleep = 0;

	err_t res = con->onSent(len);
	con->checkSelfFree();
	//debugf("<staticOnSent");
	return res;
}

err_t TcpConnection::staticOnPoll(void *arg, tcp_pcb *tcp)
{
	TcpConnection* con = (TcpConnection*)arg;

	if (con == NULL)
	{
		closeTcpConnection(tcp);
		return ERR_OK;
	}

	//if (tcp->state != ESTABLISHED)
	//	return ERR_OK;

	con->sleep++;
	err_t res = con->onPoll();
	con->checkSelfFree();
	//debugf("<staticOnPoll");
	return res;
}

void TcpConnection::staticOnError(void *arg, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	if (con == NULL) return;

	con->tcp = NULL; // IMPORTANT. No available connection after error!
	con->onError(err);
	con->checkSelfFree();
	//debugf("<staticOnError");
}

void TcpConnection::staticDnsResponse(const char *name, ip_addr_t *ipaddr, void *arg)
{
	DnsLookup* dlook = (DnsLookup*)arg;
	if (dlook == NULL) return;

	if (ipaddr != NULL)
	{
		IPAddress ip = *ipaddr;
		debugf("DNS record found: %s = %d.%d.%d.%d",
				name, ip[0], ip[1], ip[2], ip[3]);

		dlook->con->internalTcpConnect(ip, dlook->port);
	}
	else
	{
		#ifdef NETWORK_DEBUG
		debugf("DNS record _not_ found: %s", name);
		#endif

		closeTcpConnection(dlook->con->tcp);
		dlook->con->tcp = NULL;
		dlook->con->close();
	}

	delete dlook;
}

void TcpConnection::addSslOptions(uint32_t sslOptions) {
	this->sslOptions |= sslOptions;
}

boolean TcpConnection::setSslFingerprint(const uint8_t *data, int length /* = 20 */) {
	if(sslFingerprint) {
		delete[] sslFingerprint;
	}
	sslFingerprint = new uint8_t[length];
	if(sslFingerprint == NULL) {
		return false;
	}

	memcpy(sslFingerprint, data, length);
	return true;
}

boolean TcpConnection::setSslClientKeyCert(const uint8_t *key, int keyLength,
							 const uint8_t *certificate, int certificateLength,
							 const char *keyPassword /* = NULL */, boolean freeAfterHandshake /* = false */) {


	clientKeyCert.key = new uint8_t[keyLength];
	clientKeyCert.certificate = new uint8_t[certificateLength];
	int passwordLength = 0;
	if(keyPassword != NULL) {
		passwordLength = strlen(keyPassword);
		clientKeyCert.keyPassword = new char[passwordLength+1];
	}

	if(!(clientKeyCert.key && clientKeyCert.certificate &&
	    (passwordLength==0 || (passwordLength!=0 && clientKeyCert.keyPassword)))) {
		return false;
	}

	memcpy(clientKeyCert.key, key, keyLength);
	memcpy(clientKeyCert.certificate, certificate, certificateLength);
	memcpy(clientKeyCert.keyPassword, keyPassword, passwordLength);
	freeClientKeyCert = freeAfterHandshake;

	clientKeyCert.keyLength = keyLength;
	clientKeyCert.certificateLength = certificateLength;
	clientKeyCert.keyLength = keyLength;

	return true;
}

void TcpConnection::freeSslClientKeyCert() {
	if(clientKeyCert.key) {
		delete[] clientKeyCert.key;
		clientKeyCert.key = NULL;
	}

	if(clientKeyCert.certificate) {
		delete[] clientKeyCert.certificate;
		clientKeyCert.certificate = NULL;
	}

	if(clientKeyCert.keyPassword) {
		delete[] clientKeyCert.keyPassword;
		clientKeyCert.keyPassword = NULL;
	}

	clientKeyCert.keyLength = 0;
	clientKeyCert.certificateLength = 0;
}

#ifdef ENABLE_SSL
SSL* TcpConnection::getSsl() {
	return ssl;
}
#endif
