/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpConnection.h"

#include "../Data/Stream/DataSourceStream.h"
#include "../../SmingCore/Platform/WDT.h"
#include "NetUtils.h"
#include "../Wiring/WString.h"
#include "../Wiring/IPAddress.h"

#include <algorithm>

TcpConnection::TcpConnection(bool autoDestruct) : autoSelfDestruct(autoDestruct), sleep(0), canSend(true), timeOut(70)
{

}

TcpConnection::TcpConnection(tcp_pcb* connection, bool autoDestruct) : autoSelfDestruct(autoDestruct), sleep(0), canSend(true), timeOut(70)
{
	initialize(connection);
}

TcpConnection::~TcpConnection()
{
	autoSelfDestruct = false;
	close();

#ifdef ENABLE_SSL
	freeSslKeyCert();
#endif
	debug_d("~TCP connection");

	if(destroyedDelegate) {
		destroyedDelegate(*this);
	}
}

bool TcpConnection::connect(const String& server, int port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (tcp == NULL)
		initialize(tcp_new());

	ip_addr_t addr;

	this->useSsl = useSsl;
#ifdef ENABLE_SSL
	this->sslOptions |= sslOptions;

	if(sslExtension != NULL) {
		ssl_ext_free(sslExtension);
	}

	sslExtension = ssl_ext_new();
	ssl_ext_set_host_name(sslExtension, server.c_str()) ;
	ssl_ext_set_max_fragment_size(sslExtension, 4); // 4K max size
#endif

	debug_d("connect to: %s", server.c_str());
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

bool TcpConnection::connect(IPAddress addr, uint16_t port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (tcp == NULL)
		initialize(tcp_new());

	this->useSsl = useSsl;
#ifdef ENABLE_SSL
	this->sslOptions |= sslOptions;
#endif

	return internalTcpConnect(addr, port);
}

void TcpConnection::setTimeOut(uint16_t waitTimeOut)
{
	debug_d("timeout updating: %d -> %d", timeOut, waitTimeOut);
	timeOut = waitTimeOut;
}

err_t TcpConnection::onReceive(pbuf *buf)
{
	if (buf == NULL)
		debug_d("TCP received: (null)");
	else
		debug_d("TCP received: %d bytes", buf->tot_len);

	if (buf != NULL && getAvailableWriteSize() > 0)
		onReadyToSendData(eTCE_Received);

	return ERR_OK;
}

err_t TcpConnection::onSent(uint16_t len)
{
	debug_d("TCP sent: %d", len);

	//debug_d("%d %d", tcp->state, tcp->flags); // WRONG!
	if (len >= 0 && tcp != NULL && getAvailableWriteSize() > 0)
		onReadyToSendData(eTCE_Sent);

	return ERR_OK;
}

err_t TcpConnection::onPoll()
{
	if (sleep >= timeOut && timeOut != USHRT_MAX)
	{
		debug_d("TCP connection closed by timeout: %d (from %d)", sleep, timeOut);

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
		debug_d("TCP connected error status: %d", err);
	else
		debug_d("TCP connected");

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
	closeSsl();
#endif
	debug_d("TCP connection error: %d", err);
}

void TcpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if (sourceEvent != eTCE_Poll) debug_d("onReadyToSendData: %d", sourceEvent);
}

#ifdef ENABLE_SSL
err_t TcpConnection::onSslConnected(SSL *ssl)
{
	return ERR_OK;
}
#endif

int TcpConnection::writeString(const String& data, uint8_t apiflags /* = TCP_WRITE_FLAG_COPY*/)
{
	return writeString(data.c_str(), apiflags);
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
		u16_t expected = ssl_calculate_write_length(ssl, len);
		u16_t available = tcp ? tcp_sndbuf(tcp) : 0;
//		debug_d("SSL: Expected: %d, Available: %d", expected, available);
		if (expected < 0 || available < expected) {
			return -1; // No memory
		}

		int written = axl_ssl_write(ssl, (const uint8_t *)data, len);
		// debug_d("SSL: Write len: %d, Written: %d", len, written);
		if(written < ERR_OK) {
			err = written;
			debug_d("SSL: Write Error: %d", err);
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
		//debug_d("TCP connection send: %d (%d)", len, original);
		return len;
   } else {
		//debug_d("TCP connection failed with err %d (\"%s\")", err, lwip_strerr(err));
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
			debug_d("WAIT FOR FREE SPACE");
			flush();
			break; // don't try to send buffers if no free space available
		}

		// Join small fragments
		int pushCount = 0;
		do
		{
			pushCount++;
			int read = std::min((uint16_t)NETWORK_SEND_BUFFER_SIZE, getAvailableWriteSize());
			if (read > 0)
				available = stream->readMemoryBlock(buffer, read);
			else
				available = 0;

			if (available > 0)
			{
				int written = write(buffer, available, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
				total += written;
				stream->seek(std::max(written, 0));
				debug_d("Written: %d, Available: %d, isFinished: %d, PushCount: %d [TcpBuf: %d]", written, available, (stream->isFinished()?1:0), pushCount, tcp_sndbuf(tcp));
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
	closeSsl();
#endif

	if (tcp == NULL) return;
	debug_d("TCP connection closing");

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
	debug_d("+TCP connection");
	#endif
}

void TcpConnection::closeTcpConnection(tcp_pcb *tpcb)
{
	if (tpcb == NULL) return;

	debug_d("-TCP connection");

	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);
	tcp_accept(tpcb, NULL);

	auto err = tcp_close(tpcb);
	if (err != ERR_OK)
	{
		debug_d("tcp wait close connection");
		/* error closing, try again later in poll */
		tcp_poll(tpcb, staticOnPoll, 4);
	}
}

void TcpConnection::flush()
{
	if (tcp && tcp->state == ESTABLISHED)
	{
		//debug_d("TCP flush()");
		tcp_output(tcp);
	}
}

bool TcpConnection::internalTcpConnect(IPAddress addr, uint16_t port)
{
	NetUtils::FixNetworkRouting();
	err_t res = tcp_connect(tcp, addr, port, staticOnConnected);
	debug_d("TcpConnection::connect result:, %d", res);
	return res == ERR_OK;
}

err_t TcpConnection::staticOnConnected(void *arg, tcp_pcb *tcp, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	if (con == NULL)
	{
		debug_d("OnConnected ABORT");
		//closeTcpConnection(tcp);
		tcp_abort(tcp);
		return ERR_ABRT;
	}
	else
		debug_d("OnConnected");

#ifndef ENABLE_SSL
	if(con->useSsl) {
		debug_w("WARNING: SSL is not compiled. Make sure to compile Sming with 'make ENABLE_SSL=1' ");
	}
#else
	debug_d("staticOnConnected: useSSL: %d, Error: %d", con->useSsl, err);

	if(con->useSsl && err == ERR_OK) {
		int clientfd = axl_append(tcp);
		if(clientfd == -1) {
			debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
				return ERR_OK;
		}
		else {
			uint32_t sslOptions = con->sslOptions;
#ifdef SSL_DEBUG
			sslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
			debug_d("SSL: Show debug data ...");
#endif
			debug_d("SSL: Starting connection...");
#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching to 160 MHz");
			System.setCpuFrequency(eCF_160MHz); // For shorter waiting time, more power consumption.
#endif
			debug_d("SSL: handshake start (%d ms)", millis());

			if(con->sslContext != NULL) {
			    ssl_ctx_free(con->sslContext);
			}

			con->sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | sslOptions, 1);

			if (con->sslKeyCert.keyLength && con->sslKeyCert.certificateLength) {
				// if we have client certificate -> try to use it.
				if (ssl_obj_memory_load(con->sslContext, SSL_OBJ_RSA_KEY,
						con->sslKeyCert.key, con->sslKeyCert.keyLength,
						con->sslKeyCert.keyPassword) != SSL_OK) {
					debug_d("SSL: Unable to load client private key");
				} else if (ssl_obj_memory_load(con->sslContext, SSL_OBJ_X509_CERT,
						con->sslKeyCert.certificate,
						con->sslKeyCert.certificateLength, NULL) != SSL_OK) {
					debug_d("SSL: Unable to load client certificate");
				}

				if(con->freeKeyCert) {
					con->freeSslKeyCert();
				}
			}

			debug_d("SSL: Session Id Length: %d", (con->sslSessionId != NULL ? con->sslSessionId->length: 0));
			if(con->sslSessionId != NULL &&  con->sslSessionId->length > 0) {
				debug_d("-----BEGIN SSL SESSION PARAMETERS-----");
				for (int i = 0; i <  con->sslSessionId->length; i++) {
					m_printf("%02x", con->sslSessionId->value[i]);
				}

				debug_d("\n-----END SSL SESSION PARAMETERS-----");
			}

			con->ssl = ssl_client_new(con->sslContext, clientfd,
									 	 (con->sslSessionId != NULL ? con->sslSessionId->value : NULL),
										 (con->sslSessionId != NULL ? con->sslSessionId->length: 0),
										 con->sslExtension
									 );
			if(ssl_handshake_status(con->ssl)!=SSL_OK) {
				debug_d("SSL: handshake is in progress...");
				return SSL_OK;
			}

#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching back 80 MHz");
			System.setCpuFrequency(eCF_80MHz);
#endif
			if(con->sslSessionId) {
				if(con->sslSessionId->value == NULL) {
					con->sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
				}
				memcpy((void *)con->sslSessionId->value, (void *)con->ssl->session_id, con->ssl->sess_id_size);
				con->sslSessionId->length = con->ssl->sess_id_size;
			}

		}
	}
#endif

	err_t res = con->onConnected(err);
	con->checkSelfFree();
	//debug_d("<staticOnConnected");
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
		debug_d("Received ERROR %d", err);
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
		debug_d("TcpConnection::staticOnReceive: pbuf is NULL");
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
			debug_d("SSL: Got error: %d", read_bytes);
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
				debug_d("SSL: Handshake done (%d ms).", millis());
#ifndef SSL_SLOW_CONNECT
				debug_d("SSL: Switching back to 80 MHz");
				System.setCpuFrequency(eCF_80MHz); // Preserve some CPU cycles
#endif
				if(con->onSslConnected(con->ssl) != ERR_OK) {
					con->close();
					closeTcpConnection(tcp);

					return ERR_ABRT;
				}

				if(con->sslSessionId) {
					if(con->sslSessionId->value == NULL) {
						con->sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
					}
					memcpy((void *)con->sslSessionId->value, (void *)con->ssl->session_id, con->ssl->sess_id_size);
					con->sslSessionId->length = con->ssl->sess_id_size;
				}

				err_t res = con->onConnected(err);
				con->checkSelfFree();

				return res;
			}

			// No data yet
			return ERR_OK;
		}

		// we got some decrypted bytes...
		debug_d("SSL: Decrypted data len %d", read_bytes);

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
	//debug_d("<staticOnReceive");
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
	//debug_d("<staticOnSent");
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
	//debug_d("<staticOnPoll");
	return res;
}

void TcpConnection::staticOnError(void *arg, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	if (con == NULL) return;

	con->tcp = NULL; // IMPORTANT. No available connection after error!
	con->onError(err);
	con->checkSelfFree();
	//debug_d("<staticOnError");
}

void TcpConnection::staticDnsResponse(const char *name, LWIP_IP_ADDR_T *ipaddr, void *arg)
{
	DnsLookup* dlook = (DnsLookup*)arg;
	if (dlook == NULL) return;

	if (ipaddr != NULL)
	{
		IPAddress ip = *ipaddr;
		debug_d("DNS record found: %s = %d.%d.%d.%d",
				name, ip[0], ip[1], ip[2], ip[3]);

		dlook->con->internalTcpConnect(ip, dlook->port);
	}
	else
	{
		#ifdef NETWORK_DEBUG
		debug_d("DNS record _not_ found: %s", name);
		#endif

		closeTcpConnection(dlook->con->tcp);
		dlook->con->tcp = NULL;
		dlook->con->close();
	}

	delete dlook;
}

void TcpConnection::setDestroyedDelegate(TcpConnectionDestroyedDelegate destroyedDelegate)
{
	this->destroyedDelegate = destroyedDelegate;
}

#ifdef ENABLE_SSL
void TcpConnection::addSslOptions(uint32_t sslOptions)
{
	this->sslOptions |= sslOptions;
}

bool TcpConnection::setSslKeyCert(const uint8_t *key, int keyLength,
							 const uint8_t *certificate, int certificateLength,
							 const char *keyPassword /* = NULL */, bool freeAfterHandshake /* = false */)
{
	delete[] sslKeyCert.key;
	delete[] sslKeyCert.certificate;
	delete[] sslKeyCert.keyPassword;
	sslKeyCert.keyPassword = NULL;

	sslKeyCert.key = new uint8_t[keyLength];
	sslKeyCert.certificate = new uint8_t[certificateLength];
	int passwordLength = 0;
	if(keyPassword != NULL) {
		passwordLength = strlen(keyPassword);
		sslKeyCert.keyPassword = new char[passwordLength+1];
	}

	if(!(sslKeyCert.key && sslKeyCert.certificate &&
		(passwordLength==0 || sslKeyCert.keyPassword))) {
		return false;
	}

	memcpy(sslKeyCert.key, key, keyLength);
	memcpy(sslKeyCert.certificate, certificate, certificateLength);
	if(keyPassword != NULL) {
		memcpy(sslKeyCert.keyPassword, keyPassword, passwordLength);
	}
	freeKeyCert = freeAfterHandshake;

	sslKeyCert.keyLength = keyLength;
	sslKeyCert.certificateLength = certificateLength;
	sslKeyCert.keyLength = keyLength;

	return true;
}

bool TcpConnection::setSslKeyCert(const SSLKeyCertPair& keyCertPair, bool freeAfterHandshake /* = false */)
{
	this->sslKeyCert = keyCertPair;
	freeKeyCert = freeAfterHandshake;

	return true;
}

void TcpConnection::freeSslKeyCert()
{
	if(sslKeyCert.key) {
		delete[] sslKeyCert.key;
		sslKeyCert.key = NULL;
	}

	if(sslKeyCert.certificate) {
		delete[] sslKeyCert.certificate;
		sslKeyCert.certificate = NULL;
	}

	if(sslKeyCert.keyPassword) {
		delete[] sslKeyCert.keyPassword;
		sslKeyCert.keyPassword = NULL;
	}

	sslKeyCert.keyLength = 0;
	sslKeyCert.certificateLength = 0;
}

SSL* TcpConnection::getSsl()
{
	return ssl;
}

void TcpConnection::closeSsl()
{
	if (ssl == nullptr) {
		return;
	}

	debug_d("SSL: closing ...");
	ssl_ctx_free(sslContext);
	sslContext   = nullptr;
	sslExtension = nullptr;
	ssl          = nullptr;
	sslConnected = false;
}
#endif
