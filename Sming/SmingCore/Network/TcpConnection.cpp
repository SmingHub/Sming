/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpConnection.h"

#include "Data/Stream/DataSourceStream.h"
#include "Platform/WDT.h"
#include "NetUtils.h"
#include "WString.h"
#include "IPAddress.h"

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

bool TcpConnection::connect(const String& server, int port, bool useSsl, uint32_t sslOptions)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

	ip_addr_t addr;

	this->useSsl = useSsl;
#ifdef ENABLE_SSL
	this->sslOptions |= sslOptions;

	if(useSsl) {
		if(sslExtension != nullptr) {
			ssl_ext_free(sslExtension);
		}

		sslExtension = ssl_ext_new();
		ssl_ext_set_host_name(sslExtension, server.c_str());
		ssl_ext_set_max_fragment_size(sslExtension, 4); // 4K max size
	}
#endif

	debug_d("connect to: %s", server.c_str());
	canSend = false; // Wait for connection
	DnsLookup* look = new DnsLookup{this, port};
	err_t dnslook = dns_gethostbyname(server.c_str(), &addr, staticDnsResponse, look);
	if(dnslook == ERR_INPROGRESS) {
		// Operation pending - see tcpOnDnsResponse()
		return true;
	}
	delete look;

	return (dnslook == ERR_OK) ? internalTcpConnect(addr, port) : false;
}

bool TcpConnection::connect(IPAddress addr, uint16_t port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

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

err_t TcpConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		debug_d("TCP received: (null)");
	} else {
		debug_d("TCP received: %d bytes", buf->tot_len);
		if(getAvailableWriteSize() > 0) {
			onReadyToSendData(eTCE_Received);
		}
	}

	return ERR_OK;
}

err_t TcpConnection::onSent(uint16_t len)
{
	debug_d("TCP sent: %d", len);

	//debug_d("%d %d", tcp->state, tcp->flags); // WRONG!
	if(len >= 0 && tcp != nullptr && getAvailableWriteSize() > 0) {
		onReadyToSendData(eTCE_Sent);
	}

	return ERR_OK;
}

err_t TcpConnection::onPoll()
{
	if(sleep >= timeOut && timeOut != USHRT_MAX) {
		debug_d("TCP connection closed by timeout: %d (from %d)", sleep, timeOut);

		close();
		return ERR_TIMEOUT;
	}

	if(tcp != nullptr && getAvailableWriteSize() > 0) { //(tcp->state >= SYN_SENT && tcp->state <= ESTABLISHED))
		onReadyToSendData(eTCE_Poll);
	}

	return ERR_OK;
}

err_t TcpConnection::onConnected(err_t err)
{
	if(err != ERR_OK) {
		debug_d("TCP connected error status: %d", err);
	} else {
		debug_d("TCP connected");
	}

	canSend = true;
	if(err == ERR_OK) {
		onReadyToSendData(eTCE_Connected);
	} else {
		close();
	}

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
	if(sourceEvent != eTCE_Poll) {
		debug_d("onReadyToSendData: %d", sourceEvent);
	}
}

#ifdef ENABLE_SSL
err_t TcpConnection::onSslConnected(SSL* ssl)
{
	return ERR_OK;
}
#endif

int TcpConnection::write(const char* data, int len, uint8_t apiflags /* = TCP_WRITE_FLAG_COPY*/)
{
	WDT.alive();

	err_t err = ERR_OK;

#ifdef ENABLE_SSL
	if(ssl) {
		u16_t expected = ssl_calculate_write_length(ssl, len);
		u16_t available = tcp ? tcp_sndbuf(tcp) : 0;
		//		debug_d("SSL: Expected: %d, Available: %d", expected, available);
		if(expected < 0 || available < expected) {
			return -1; // No memory
		}

		int written = axl_ssl_write(ssl, (const uint8_t*)data, len);
		// debug_d("SSL: Write len: %d, Written: %d", len, written);
		if(written < ERR_OK) {
			err = written;
			debug_d("SSL: Write Error: %d", err);
		}
	} else {
#endif
		u16_t available = getAvailableWriteSize();
		if(available < len) {
			if(available == 0) {
				return -1; // No memory
			} else {
				len = available;
			}
		}
		err = tcp_write(tcp, data, len, apiflags);

#ifdef ENABLE_SSL
	}
#endif

	if(err == ERR_OK) {
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
	if(ssl != nullptr && !sslConnected) {
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

	do {
		space = (tcp_sndqueuelen(tcp) < TCP_SND_QUEUELEN);
		if(!space) {
			debug_d("WAIT FOR FREE SPACE");
			flush();
			break; // don't try to send buffers if no free space available
		}

		// Join small fragments
		int pushCount = 0;
		do {
			pushCount++;
			int read = std::min((uint16_t)NETWORK_SEND_BUFFER_SIZE, getAvailableWriteSize());
			if(read > 0) {
				available = stream->readMemoryBlock(buffer, read);
			} else {
				available = 0;
			}

			if(available > 0) {
				int written = write(buffer, available, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
				total += written;
				stream->seek(std::max(written, 0));
				debug_d("Written: %d, Available: %d, isFinished: %d, PushCount: %d [TcpBuf: %d]", written, available,
						(stream->isFinished() ? 1 : 0), pushCount, tcp_sndbuf(tcp));
				repeat = written == available && !stream->isFinished() && pushCount < 25;
			} else {
				repeat = false;
			}
		} while(repeat);

		space = (tcp_sndqueuelen(tcp) < TCP_SND_QUEUELEN); // && tcp_sndbuf(tcp) >= FILE_STREAM_BUFFER_SIZE;
	} while(repeat && space);

	flush();
	return total;
}

void TcpConnection::close()
{
#ifdef ENABLE_SSL
	closeSsl();
#endif

	if(tcp == nullptr) {
		return;
	}
	debug_d("TCP connection closing");

#ifdef ENABLE_SSL
	axl_free(tcp);
#endif

	tcp_poll(tcp, staticOnPoll, 1);
	tcp_arg(tcp, nullptr); // reset pointer to close connection on next callback
	tcp = nullptr;

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

void TcpConnection::closeTcpConnection(tcp_pcb* tpcb)
{
	if(tpcb == nullptr) {
		return;
	}

	debug_d("-TCP connection");

	tcp_arg(tpcb, nullptr);
	tcp_sent(tpcb, nullptr);
	tcp_recv(tpcb, nullptr);
	tcp_err(tpcb, nullptr);
	tcp_poll(tpcb, nullptr, 0);
	tcp_accept(tpcb, nullptr);

	auto err = tcp_close(tpcb);
	if(err != ERR_OK) {
		debug_d("tcp wait close connection");
		/* error closing, try again later in poll */
		tcp_poll(tpcb, staticOnPoll, 4);
	}
}

void TcpConnection::flush()
{
	if(tcp && tcp->state == ESTABLISHED) {
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

err_t TcpConnection::staticOnConnected(void* arg, tcp_pcb* tcp, err_t err)
{
	auto con = static_cast<TcpConnection*>(arg);
	if(con == nullptr) {
		debug_d("OnConnected ABORT");
		//closeTcpConnection(tcp);
		tcp_abort(tcp);
		return ERR_ABRT;
	} else {
		return con->tcpOnConnected(err);
	}
}

err_t TcpConnection::tcpOnConnected(err_t err)
{
	debug_d("OnConnected");

#ifndef ENABLE_SSL
	if(useSsl) {
		debug_w("WARNING: SSL is not compiled. Make sure to compile Sming with 'make ENABLE_SSL=1' ");
	}
#else
	debug_d("tcpOnConnected: useSSL: %d, Error: %d", useSsl, err);

	if(useSsl && err == ERR_OK) {
		int clientfd = axl_append(tcp);
		if(clientfd < 0) {
			debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
			return ERR_OK;
		} else {
			uint32_t localSslOptions = sslOptions;
#ifdef SSL_DEBUG
			localSslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
			debug_d("SSL: Show debug data ...");
#endif
			debug_d("SSL: Starting connection...");
#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching to 160 MHz");
			System.setCpuFrequency(eCF_160MHz); // For shorter waiting time, more power consumption.
#endif
			debug_d("SSL: handshake start (%d ms)", millis());

			ssl_ctx_free(sslContext);
			sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | localSslOptions, 1);

			if(sslKeyCert.keyLength != 0 && sslKeyCert.certificateLength != 0) {
				// if we have client certificate -> try to use it.
				if(ssl_obj_memory_load(sslContext, SSL_OBJ_RSA_KEY, sslKeyCert.key, sslKeyCert.keyLength,
									   sslKeyCert.keyPassword) != SSL_OK) {
					debug_d("SSL: Unable to load client private key");
				} else if(ssl_obj_memory_load(sslContext, SSL_OBJ_X509_CERT, sslKeyCert.certificate,
											  sslKeyCert.certificateLength, nullptr) != SSL_OK) {
					debug_d("SSL: Unable to load client certificate");
				}

				if(freeKeyCert) {
					freeSslKeyCert();
				}
			}

			debug_d("SSL: Session Id Length: %d", sslSessionId != nullptr ? sslSessionId->length : 0);
			if(sslSessionId != nullptr && sslSessionId->length > 0) {
				debug_d("-----BEGIN SSL SESSION PARAMETERS-----");
				debug_hex(DBG, "Session", sslSessionId->value, sslSessionId->length);
				debug_d("\n-----END SSL SESSION PARAMETERS-----");
			}

			ssl = ssl_client_new(sslContext, clientfd, sslSessionId != nullptr ? sslSessionId->value : nullptr,
								 sslSessionId != nullptr ? sslSessionId->length : 0, sslExtension);
			if(ssl_handshake_status(ssl) != SSL_OK) {
				debug_d("SSL: handshake is in progress...");
				return SSL_OK;
			}

#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching back 80 MHz");
			System.setCpuFrequency(eCF_80MHz);
#endif
			if(sslSessionId != nullptr) {
				if(sslSessionId->value == nullptr) {
					sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
				}
				memcpy(sslSessionId->value, ssl->session_id, ssl->sess_id_size);
				sslSessionId->length = ssl->sess_id_size;
			}
		}
	}
#endif

	err_t res = onConnected(err);
	checkSelfFree();
	//debug_d("<tcpOnConnected");
	return res;
}

err_t TcpConnection::staticOnReceive(void* arg, tcp_pcb* tcp, pbuf* p, err_t err)
{
	auto con = static_cast<TcpConnection*>(arg);
	//Serial.println("echo_recv!");

	if(con == nullptr) {
		if(p != nullptr) {
			/* Inform TCP that we have taken the data. */
			tcp_recved(tcp, p->tot_len);
			pbuf_free(p);
		}
		closeTcpConnection(tcp);
		return ERR_OK;
	} else {
		return con->tcpOnReceive(p, err);
	}
}

err_t TcpConnection::tcpOnReceive(pbuf* p, err_t err)
{
	sleep = 0;

	if(err != ERR_OK /*&& err != ERR_CLSD && err != ERR_RST*/) {
		debug_d("Received ERROR %d", err);
		/* exit and free resources, for unknown reason */
		if(p != nullptr) {
			/* Inform TCP that we have taken the data. */
			tcp_recved(tcp, p->tot_len);
			pbuf_free(p);
		}
		closeTcpConnection(tcp); // ??
		tcp = nullptr;
		onError(err);
		//close();
		return err == ERR_ABRT ? ERR_ABRT : ERR_OK;
	}

	//if (tcp != nullptr && tcp->state == ESTABLISHED) // If active
	/* We have taken the data. */
	if(p != nullptr) {
		tcp_recved(tcp, p->tot_len);
	} else {
		debug_d("TcpConnection::tcpOnReceive: pbuf is NULL");
	}

#ifdef ENABLE_SSL
	if(ssl != nullptr && p != nullptr) {
		WDT.alive(); /* SSL handshake needs time. In theory we have max 8 seconds before the hardware watchdog resets the device */

		struct pbuf* pout;
		int read_bytes = axl_ssl_read(ssl, tcp, p, &pout);

		// free the SSL pbuf and put the decrypted data in the brand new pout pbuf
		if(p != nullptr) {
			pbuf_free(p);
		}

		if(read_bytes < SSL_OK) {
			debug_d("SSL: Got error: %d", read_bytes);
			if(read_bytes == SSL_CLOSE_NOTIFY) {
				return ERR_OK;
			}

			close();
			closeTcpConnection(tcp);
			return read_bytes;
		}

		if(read_bytes == 0) {
			if(!sslConnected && ssl_handshake_status(ssl) == SSL_OK) {
				sslConnected = true;
				debug_d("SSL: Handshake done (%d ms).", millis());
#ifndef SSL_SLOW_CONNECT
				debug_d("SSL: Switching back to 80 MHz");
				System.setCpuFrequency(eCF_80MHz); // Preserve some CPU cycles
#endif
				if(onSslConnected(ssl) != ERR_OK) {
					close();
					closeTcpConnection(tcp);

					return ERR_ABRT;
				}

				if(sslSessionId != nullptr) {
					if(sslSessionId->value == nullptr) {
						sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
					}
					assert(ssl->sess_id_size <= SSL_SESSION_ID_SIZE);
					memcpy(sslSessionId->value, ssl->session_id, ssl->sess_id_size);
					sslSessionId->length = ssl->sess_id_size;
				}

				err_t res = onConnected(err);
				checkSelfFree();

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

	err_t res = onReceive(p);

	if(p != nullptr) {
		pbuf_free(p);
	} else {
		close();
		closeTcpConnection(tcp);
	}

	checkSelfFree();
	//debug_d("<tcpOnReceive");
	return res;
}

err_t TcpConnection::staticOnSent(void* arg, tcp_pcb* tcp, uint16_t len)
{
	auto con = static_cast<TcpConnection*>(arg);

	if(con == nullptr) {
		return ERR_OK;
	} else {
		return con->tcpOnSent(len);
	}
}

err_t TcpConnection::tcpOnSent(uint16_t len)
{
	sleep = 0;
	err_t res = onSent(len);
	checkSelfFree();
	//debug_d("<tcpOnSent");
	return res;
}

err_t TcpConnection::staticOnPoll(void* arg, tcp_pcb* tcp)
{
	auto con = static_cast<TcpConnection*>(arg);

	if(con == nullptr) {
		closeTcpConnection(tcp);
		return ERR_OK;
	} else {
		return con->tcpOnPoll();
	}
}

err_t TcpConnection::tcpOnPoll()
{
	//if (tcp->state != ESTABLISHED)
	//	return ERR_OK;

	sleep++;
	err_t res = onPoll();
	checkSelfFree();
	//debug_d("<tcpOnPoll");
	return res;
}

void TcpConnection::staticOnError(void* arg, err_t err)
{
	auto con = static_cast<TcpConnection*>(arg);

	if(con != nullptr) {
		con->tcpOnError(err);
	}
}

void TcpConnection::tcpOnError(err_t err)
{
	tcp = nullptr; // IMPORTANT. No available connection after error!
	onError(err);
	checkSelfFree();
	//debug_d("<staticOnError");
}

void TcpConnection::staticDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, void* arg)
{
	auto dlook = static_cast<DnsLookup*>(arg);
	if(dlook != nullptr) {
		dlook->con->tcpOnDnsResponse(name, ipaddr, dlook->port);
		delete dlook;
	}
}

void TcpConnection::tcpOnDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, int port)
{
	if(ipaddr != nullptr) {
		IPAddress ip = *ipaddr;
		debug_d("DNS record found: %s = %s", name, ip.toString().c_str());

		internalTcpConnect(ip, port);
	} else {
#ifdef NETWORK_DEBUG
		debug_d("DNS record _not_ found: %s", name);
#endif

		closeTcpConnection(tcp);
		tcp = nullptr;
		close();
	}
}

#ifdef ENABLE_SSL

bool TcpConnection::setSslKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
								  const char* keyPassword, bool freeAfterHandshake)
{
	delete[] sslKeyCert.key;
	delete[] sslKeyCert.certificate;
	delete[] sslKeyCert.keyPassword;
	sslKeyCert.keyPassword = nullptr;

	sslKeyCert.key = new uint8_t[keyLength];
	sslKeyCert.certificate = new uint8_t[certificateLength];
	int passwordLength = 0;
	if(keyPassword != nullptr) {
		passwordLength = strlen(keyPassword);
		sslKeyCert.keyPassword = new char[passwordLength + 1];
	}

	if(!(sslKeyCert.key && sslKeyCert.certificate && (passwordLength == 0 || sslKeyCert.keyPassword))) {
		return false;
	}

	memcpy(sslKeyCert.key, key, keyLength);
	memcpy(sslKeyCert.certificate, certificate, certificateLength);
	if(keyPassword != nullptr) {
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
	freeSslKeyCert();
	sslKeyCert = keyCertPair;
	freeKeyCert = freeAfterHandshake;

	return true;
}

void TcpConnection::freeSslKeyCert()
{
	if(sslKeyCert.key) {
		delete[] sslKeyCert.key;
		sslKeyCert.key = nullptr;
	}

	if(sslKeyCert.certificate) {
		delete[] sslKeyCert.certificate;
		sslKeyCert.certificate = nullptr;
	}

	if(sslKeyCert.keyPassword) {
		delete[] sslKeyCert.keyPassword;
		sslKeyCert.keyPassword = nullptr;
	}

	sslKeyCert.keyLength = 0;
	sslKeyCert.certificateLength = 0;
}

void TcpConnection::closeSsl()
{
	if(ssl == nullptr) {
		return;
	}

	debug_d("SSL: closing ...");
	ssl_ctx_free(sslContext);
	sslContext = nullptr;
	sslExtension = nullptr;
	ssl = nullptr;
	sslConnected = false;
}
#endif
