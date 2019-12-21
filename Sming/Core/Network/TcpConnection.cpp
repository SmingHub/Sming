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
#include <Network/Ssl/Factory.h>

#include <Data/Stream/DataSourceStream.h>
#include "NetUtils.h"
#include <WString.h>

#ifdef DEBUG_TCP_EXTENDED
#define debug_tcp(fmt, ...) debug_d(fmt, ##__VA_ARGS__)
#else
#define debug_tcp(fmt, ...) debug_none(fmt, ##__VA_ARGS__)
#endif

TcpConnection::~TcpConnection()
{
	autoSelfDestruct = false;
	close();

	delete ssl;

	debug_d("~TCP connection");

	if(destroyedDelegate) {
		destroyedDelegate(*this);
	}
}

bool TcpConnection::sslCreateSession()
{
	if(ssl != nullptr) {
		return true;
	}

	if(Ssl::factory == nullptr) {
		debug_e("SSL required, no factory");
		return false;
	}

	ssl = new Ssl::Session;

	return ssl != nullptr;
}

bool TcpConnection::connect(const String& server, int port, bool useSsl, uint32_t sslOptions)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

	ip_addr_t addr;

	this->useSsl = useSsl;
	if(useSsl) {
		if(!sslCreateSession()) {
			return false;
		}
		ssl->options |= sslOptions;
		ssl->extension.hostName = server;
		ssl->extension.fragmentSize = Ssl::Extension::eSEFS_4K; // 4K max size
	}

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

bool TcpConnection::connect(IpAddress addr, uint16_t port, bool useSsl, uint32_t sslOptions)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

	this->useSsl = useSsl;
	if(useSsl) {
		if(!sslCreateSession()) {
			return false;
		}
		ssl->options |= sslOptions;
	}

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
	if(ssl != nullptr) {
		ssl->close();
	}
	debug_d("TCP connection error: %d", err);
}

void TcpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if(sourceEvent != eTCE_Poll) {
		debug_d("TCP onReadyToSendData: %d", sourceEvent);
	}
}

err_t TcpConnection::onSslConnected(Ssl::Connection* connection)
{
	return ERR_OK;
}

int TcpConnection::write(const char* data, int len, uint8_t apiflags)
{
	err_t err = ERR_OK;

	if(ssl != nullptr) {
		err = ssl->write(reinterpret_cast<const uint8_t*>(data), len);
	} else {
		u16_t available = getAvailableWriteSize();
		if(available < len) {
			if(available == 0) {
				return -1; // No memory
			} else {
				len = available;
			}
		}
		err = tcp_write(tcp, data, len, apiflags);
	}

	if(err == ERR_OK) {
		debug_tcp("TCP connection send: %d", len);
		return len;
	} else {
		debug_tcp("TCP connection failed with err %d (\"%s\")", err, lwip_strerr(err));
		return -1;
	}
}

int TcpConnection::write(IDataSourceStream* stream)
{
	if(ssl != nullptr && !ssl->connected) {
		// wait until the SSL handshake is done.
		return 0;
	}

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

	if(!space) {
		flush();
	}

	return total;
}

void TcpConnection::close()
{
	if(ssl != nullptr) {
		ssl->close();
	}

	if(tcp == nullptr) {
		return;
	}
	debug_d("TCP connection closing");

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

bool TcpConnection::internalConnect(IpAddress addr, uint16_t port)
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
	debug_d("TCP connected: useSSL: %d, Error: %d", useSsl, err);

	if(useSsl && err == ERR_OK) {
		if(ssl == nullptr) {
			debug_e("Unable to create SSL connection without SSL implementation.");
			return ERR_ABRT;
		}

		auto err = ssl->onConnected(tcp);
		if(err == ERR_INPROGRESS) {
			return ERR_OK;
		}
		if(err != ERR_OK) {
			return err;
		}
	}

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

	if(ssl != nullptr && p != nullptr) {
		bool isConnecting = !ssl->connected;
		pbuf* out;
		int res = ssl->read(p, out);
		if(res < 0) {
			close();
			closeTcpConnection(tcp);
			return res;
		}
		p = out;

		if(isConnecting && ssl->connected) {
			debug_tcp("SSL Just connected, err = %d", res);
			if(onSslConnected(ssl->connection) != ERR_OK) {
				debug_tcp("onSslConnected failed");

				if(p != nullptr) {
					pbuf_free(p);
				}

				close();
				closeTcpConnection(tcp);

				return ERR_ABRT;
			}

			err = onConnected(ERR_OK);
			checkSelfFree();

			return err;
		}

		// No data received
		if(res == ERR_OK) {
			return err;
		}

		// Proceed with received decrypted data
	}

	err = onReceive(p);

	if(p != nullptr) {
		pbuf_free(p);
		checkSelfFree();
	} else {
		close();
	}

	debug_tcp("<TCP receive");
	return err;
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
		IpAddress ip = *ipaddr;
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
