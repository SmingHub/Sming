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

#define debug_tcp_e(fmt, ...) debug_e("TCP %p " fmt, this, ##__VA_ARGS__)
#define debug_tcp_w(fmt, ...) debug_w("TCP %p " fmt, this, ##__VA_ARGS__)
#define debug_tcp_i(fmt, ...) debug_i("TCP %p " fmt, this, ##__VA_ARGS__)
#define debug_tcp_d(fmt, ...) debug_d("TCP %p " fmt, this, ##__VA_ARGS__)

#ifdef DEBUG_TCP_EXTENDED
#define debug_tcp_ext debug_tcp_d(fmt, ##__VA_ARGS__)
#else
#define debug_tcp_ext(fmt, ...) debug_none(fmt, ##__VA_ARGS__)
#endif

TcpConnection::~TcpConnection()
{
	autoSelfDestruct = false;
	close();

	delete ssl;

	debug_tcp_d("~connection");

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
		debug_tcp_e("SSL required, no factory");
		return false;
	}

	ssl = new Ssl::Session;
	if(ssl == nullptr) {
		return false;
	}

	sslInitSession(*ssl);
	return true;
}

bool TcpConnection::connect(const String& server, int port, bool useSsl)
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
		ssl->hostName = server;
	}

	debug_tcp_d("connect to \"%s\"", server.c_str());
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

bool TcpConnection::connect(IpAddress addr, uint16_t port, bool useSsl)
{
	if(tcp == nullptr) {
		initialize(tcp_new());
	}

	this->useSsl = useSsl;
	if(useSsl && !sslCreateSession()) {
		return false;
	}

	return internalConnect(addr, port);
}

void TcpConnection::setTimeOut(uint16_t waitTimeOut)
{
	debug_tcp_d("timeout updating: %d -> %d", timeOut, waitTimeOut);
	timeOut = waitTimeOut;
}

err_t TcpConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		debug_tcp_d("received: (null)");
	} else {
		debug_tcp_d("received: %d bytes", buf->tot_len);
		trySend(eTCE_Received);
	}

	return ERR_OK;
}

err_t TcpConnection::onSent(uint16_t len)
{
	debug_tcp_d("sent: %u", len);

	if(tcp != nullptr) {
		debug_tcp_ext("%d %d", tcp->state, tcp->flags); // WRONG!
		trySend(eTCE_Sent);
	}

	return ERR_OK;
}

err_t TcpConnection::onPoll()
{
	if(sleep >= timeOut && timeOut != USHRT_MAX) {
		debug_tcp_d("connection closed by timeout: %d (from %d)", sleep, timeOut);

		close();
		return ERR_TIMEOUT;
	}

	trySend(eTCE_Poll);

	return ERR_OK;
}

err_t TcpConnection::onConnected(err_t err)
{
	if(err != ERR_OK) {
		debug_tcp_d("connected error status: %d", err);
	} else {
		debug_tcp_d("connected");
	}

	canSend = true;
	if(err == ERR_OK) {
		trySend(eTCE_Connected);
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
	debug_tcp_d("connection error: %d", err);
}

void TcpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	if(sourceEvent != eTCE_Poll) {
		debug_tcp_d("onReadyToSendData: %d", sourceEvent);
	}
}

int TcpConnection::write(const char* data, int len, uint8_t apiflags)
{
	err_t err;

	if(ssl != nullptr) {
		len = ssl->write(reinterpret_cast<const uint8_t*>(data), len);
		err = (len < 0) ? len : ERR_OK;
	} else {
		u16_t available = getAvailableWriteSize();
		if(available < len) {
			if(available == 0) {
				return ERR_MEM;
			}

			len = available;
		}

		err = tcp_write(tcp, data, len, apiflags);
	}

	if(err < 0) {
		debug_tcp_ext("connection failed with err %d (\"%s\")", err, lwip_strerr(err));
		return err;
	}

	debug_tcp_ext("connection send: %d", len);
	return len;
}

int TcpConnection::write(IDataSourceStream* stream)
{
	if(ssl != nullptr && !ssl->isConnected()) {
		// wait until the SSL handshake is done.
		return 0;
	}

	// Send data from DataStream
	size_t total = 0;
	unsigned pushCount = 0;
	while((tcp_sndqueuelen(tcp) < TCP_SND_QUEUELEN) && !stream->isFinished() && (pushCount < 25)) {
		size_t available = getAvailableWriteSize();
		if(available == 0) {
			break;
		}

		char buffer[NETWORK_SEND_BUFFER_SIZE];
		auto bytesRead = stream->readMemoryBlock(buffer, std::min(sizeof(buffer), available));
		if(bytesRead == 0) {
			break;
		}

		++pushCount;

		int bytesWritten = write(buffer, bytesRead, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
		debug_tcp_d("Written: %d, Available: %u, isFinished: %d, PushCount: %u", bytesWritten, available,
					stream->isFinished(), pushCount);
		if(bytesWritten <= 0) {
			continue;
		}

		if(bytesWritten > 0) {
			total += size_t(bytesWritten);
			stream->seek(bytesWritten);
		}
	}

	if(pushCount == 0) {
		debug_tcp_d("WAIT FOR FREE SPACE");
	} else {
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
	debug_tcp_d("connection closing");

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
	debug_tcp_d("+connection");
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
		debug_tcp_ext("flush()");
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
	debug_tcp_d("connected: useSSL: %d, Error: %d", useSsl, err);

	if(useSsl && err == ERR_OK) {
		if(ssl == nullptr) {
			debug_tcp_e("SSL disabled: aborting connection");
			return ERR_ABRT;
		}
		if(!ssl->onConnect(tcp)) {
			return ERR_ABRT;
		}
		if(!ssl->isConnected()) {
			return ERR_OK;
		}
	}

	err_t res = onConnected(err);
	checkSelfFree();
	debug_tcp_ext("<connected");
	return res;
}

err_t TcpConnection::internalOnReceive(pbuf* p, err_t err)
{
	sleep = 0;

	if(err != ERR_OK /*&& err != ERR_CLSD && err != ERR_RST*/) {
		debug_tcp_d("receive ERROR %d", err);
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
		debug_tcp_d("receive: pbuf is NULL");
	}

	if(ssl != nullptr && p != nullptr) {
		bool isConnecting = !ssl->isConnected();

		Ssl::InputBuffer input(p);

		pbuf pbufOut = {};
		while(input.available() > 0) {
			uint8_t* output;
			int len = ssl->read(input, output);
			if(len < 0) {
				close();
				closeTcpConnection(tcp);
				return ERR_ABRT;
			}

			if(isConnecting && ssl->isConnected()) {
				err = onConnected(ERR_OK);
			} else if(len != 0) {
				// Proceed with received decrypted data
				pbufOut.payload = output;
				pbufOut.tot_len = len;
				pbufOut.len = len;
				err = onReceive(&pbufOut);
			}

			if(err < 0) {
				break;
			}
		}

	} else {
		err = onReceive(p);
	}

	if(p != nullptr) {
		pbuf_free(p);
		checkSelfFree();
	} else {
		close();
	}

	debug_tcp_ext("<receive");
	return err;
}

err_t TcpConnection::internalOnSent(uint16_t len)
{
	sleep = 0;
	err_t res = onSent(len);
	checkSelfFree();
	debug_tcp_ext("<sent");
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
	sleep++;
	err_t res = onPoll();
	if(res == ERR_OK) {
		checkSelfFree();
	}
	debug_tcp_ext("<poll");
	return res;
}

void TcpConnection::internalOnError(err_t err)
{
	tcp = nullptr; // IMPORTANT. No available connection after error!
	onError(err);
	checkSelfFree();
	debug_tcp_ext("<error");
}

void TcpConnection::internalOnDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, int port)
{
	if(ipaddr != nullptr) {
		IpAddress ip = *ipaddr;
		debug_tcp_d("DNS record found: %s = %s", name, ip.toString().c_str());

		internalConnect(ip, port);
	} else {
#ifdef NETWORK_DEBUG
		debug_tcp_d("DNS record _not_ found: %s", name);
#endif

		closeTcpConnection(tcp);
		tcp = nullptr;
		close();
	}
}
