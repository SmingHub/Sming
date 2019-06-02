/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpConnection.cpp
 *
 ****/

#include "TcpConnection.h"

#include "Data/Stream/DataSourceStream.h"
#include "Platform/WDT.h"
#include "NetUtils.h"
#include "WString.h"
#include "IPAddress.h"

#ifdef DEBUG_TCP_EXTENDED
#define debug_tcp(fmt, ...) debug_d(fmt, ##__VA_ARGS__)
#else
#define debug_tcp(fmt, ...) debug_none(fmt, ##__VA_ARGS__)
#endif

TcpConnection::~TcpConnection()
{
	autoSelfDestruct = false;
	close();

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
		ssl_ext_free(sslExtension);
		sslExtension = ssl_ext_new();
		ssl_ext_set_host_name(sslExtension, server.c_str());
		ssl_ext_set_max_fragment_size(sslExtension, 4); // 4K max size
	}
#endif

	debug_d("connect to: %s", server.c_str());
	canSend = false; // Wait for connection

	struct DnsLookup {
		TcpConnection* con;
		int port;
	};
	DnsLookup* look = new DnsLookup{this, port};
	err_t dnslook = dns_gethostbyname(server.c_str(), &addr,
									  [](const char* name, LWIP_IP_ADDR_T* ipaddr, void* arg) {
										  auto dlook = static_cast<DnsLookup*>(arg);
										  if(dlook != nullptr) {
											  dlook->con->internalOnDnsResponse(name, ipaddr, dlook->port);
											  delete dlook;
										  }
									  },
									  look);
	if(dnslook == ERR_INPROGRESS) {
		// Operation pending - see internalOnDnsResponse()
		return true;
	}
	delete look;

	return (dnslook == ERR_OK) ? internalConnect(addr, port) : false;
}

bool TcpConnection::connect(IPAddress addr, uint16_t port, bool useSsl, uint32_t sslOptions)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

	this->useSsl = useSsl;
#ifdef ENABLE_SSL
	this->sslOptions |= sslOptions;
#endif

	return internalConnect(addr, port);
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

	if(tcp != nullptr) {
		debug_tcp("%d %d", tcp->state, tcp->flags); // WRONG!

		if(getAvailableWriteSize() > 0) {
			onReadyToSendData(eTCE_Sent);
		}
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
		debug_d("TCP onReadyToSendData: %d", sourceEvent);
	}
}

#ifdef ENABLE_SSL
err_t TcpConnection::onSslConnected(SSL* ssl)
{
	return ERR_OK;
}
#endif

int TcpConnection::write(const char* data, int len, uint8_t apiflags)
{
	WDT.alive();

	err_t err = ERR_OK;

#ifdef ENABLE_SSL
	if(ssl) {
		u16_t expected = ssl_calculate_write_length(ssl, len);
		u16_t available = tcp ? tcp_sndbuf(tcp) : 0;
		debug_tcp("SSL: Expected: %d, Available: %d", expected, available);
		if(expected < 0 || available < expected) {
			return -1; // No memory
		}

		int written = axl_ssl_write(ssl, (const uint8_t*)data, len);
		debug_tcp("SSL: Write len: %d, Written: %d", len, written);
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
		debug_tcp("TCP connection send: %d (%d)", len, original);
		return len;
	} else {
		debug_tcp("TCP connection failed with err %d (\"%s\")", err, lwip_strerr(err));
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
				debug_d("TCP Written: %d, Available: %d, isFinished: %d, PushCount: %d [TcpBuf: %d]", written,
						available, (stream->isFinished() ? 1 : 0), pushCount, tcp_sndbuf(tcp));
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
	tcp_arg(tcp, this);

	tcp_sent(tcp, [](void* arg, tcp_pcb* tcp, uint16_t len) -> err_t {
		auto con = static_cast<TcpConnection*>(arg);
		return (con == nullptr) ? ERR_OK : con->internalOnSent(len);
	});

	tcp_recv(tcp, [](void* arg, tcp_pcb* tcp, pbuf* p, err_t err) -> err_t {
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
			return con->internalOnReceive(p, err);
		}
	});

	tcp_err(tcp, [](void* arg, err_t err) {
		auto con = static_cast<TcpConnection*>(arg);
		if(con != nullptr) {
			con->internalOnError(err);
		}
	});

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
		debug_tcp("TCP flush()");
		tcp_output(tcp);
	}
}

bool TcpConnection::internalConnect(IPAddress addr, uint16_t port)
{
	NetUtils::FixNetworkRouting();
	err_t res = tcp_connect(tcp, addr, port, [](void* arg, tcp_pcb* tcp, err_t err) -> err_t {
		auto con = static_cast<TcpConnection*>(arg);
		if(con == nullptr) {
			debug_d("TCP connect ABORT");
			//closeTcpConnection(tcp);
			tcp_abort(tcp);
			return ERR_ABRT;
		} else {
			return con->internalOnConnected(err);
		}
	});
	debug_d("TCP connect result: %d", res);
	return res == ERR_OK;
}

err_t TcpConnection::internalOnConnected(err_t err)
{
	debug_d("TCP connected");

#ifndef ENABLE_SSL
	if(useSsl) {
		debug_w("WARNING: SSL is not compiled. Make sure to compile Sming with 'make ENABLE_SSL=1' ");
	}
#else
	debug_d("TCP connected: useSSL: %d, Error: %d", useSsl, err);

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

			if(sslKeyCert.isValid()) {
				// if we have client certificate -> try to use it.
				if(ssl_obj_memory_load(sslContext, SSL_OBJ_RSA_KEY, sslKeyCert.getKey(), sslKeyCert.getKeyLength(),
									   sslKeyCert.getKeyPassword()) != SSL_OK) {
					debug_d("SSL: Unable to load client private key");
				} else if(ssl_obj_memory_load(sslContext, SSL_OBJ_X509_CERT, sslKeyCert.getCertificate(),
											  sslKeyCert.getCertificateLength(), nullptr) != SSL_OK) {
					debug_d("SSL: Unable to load client certificate");
				}

				if(freeKeyCertAfterHandshake) {
					sslKeyCert.free();
				}
			}

			debug_d("SSL: Session Id Length: %u", sslSessionId->getLength());
			if(sslSessionId->isValid()) {
				debug_d("-----BEGIN SSL SESSION PARAMETERS-----");
				debug_hex(DBG, "Session", sslSessionId->getValue(), sslSessionId->getLength());
				debug_d("\n-----END SSL SESSION PARAMETERS-----");
			}

			ssl =
				ssl_client_new(sslContext, clientfd, sslSessionId->getValue(), sslSessionId->getLength(), sslExtension);
			if(ssl_handshake_status(ssl) != SSL_OK) {
				debug_d("SSL: handshake is in progress...");
				return SSL_OK;
			}

#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching back 80 MHz");
			System.setCpuFrequency(eCF_80MHz);
#endif
			if(sslSessionId != nullptr) {
				sslSessionId->assign(ssl->session_id, ssl->sess_id_size);
			}
		}
	}
#endif

	err_t res = onConnected(err);
	checkSelfFree();
	debug_tcp("<TCP connected");
	return res;
}

err_t TcpConnection::internalOnReceive(pbuf* p, err_t err)
{
	sleep = 0;

	if(err != ERR_OK /*&& err != ERR_CLSD && err != ERR_RST*/) {
		debug_d("TCP receive ERROR %d", err);
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
		debug_d("TCP receive: pbuf is NULL");
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
					sslSessionId->assign(ssl->session_id, ssl->sess_id_size);
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
	debug_tcp("<TCP receive");
	return res;
}

err_t TcpConnection::internalOnSent(uint16_t len)
{
	sleep = 0;
	err_t res = onSent(len);
	checkSelfFree();
	debug_tcp("<TCP sent");
	return res;
}

err_t TcpConnection::staticOnPoll(void* arg, tcp_pcb* tcp)
{
	auto con = static_cast<TcpConnection*>(arg);

	if(con == nullptr) {
		closeTcpConnection(tcp);
		return ERR_OK;
	} else {
		return con->internalOnPoll();
	}
}

err_t TcpConnection::internalOnPoll()
{
	//if (tcp->state != ESTABLISHED)
	//	return ERR_OK;

	sleep++;
	err_t res = onPoll();
	checkSelfFree();
	debug_tcp("<TCP poll");
	return res;
}

void TcpConnection::internalOnError(err_t err)
{
	tcp = nullptr; // IMPORTANT. No available connection after error!
	onError(err);
	checkSelfFree();
	debug_tcp("<TCP error");
}

void TcpConnection::internalOnDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, int port)
{
	if(ipaddr != nullptr) {
		IPAddress ip = *ipaddr;
		debug_d("DNS record found: %s = %s", name, ip.toString().c_str());

		internalConnect(ip, port);
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
