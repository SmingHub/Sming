/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpConnection.h"

#include "Data/Stream/DataSourceStream.h"
#include "WDT.h"
#include "NetUtils.h"
#include "WString.h"
#include "IPAddress.h"

TcpConnection::~TcpConnection()
{
	_autoSelfDestruct = false;
	close();

#ifdef ENABLE_SSL
	freeSslKeyCert();
#endif
	debug_d("~TCP connection");

	if (_destroyedDelegate) {
		_destroyedDelegate(*this);
	}
}

bool TcpConnection::connect(const String& server, int port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (!_tcp)
		initialize(tcp_new());

	ip_addr_t addr;

	_useSsl = useSsl;
#ifdef ENABLE_SSL
	_sslOptions |= sslOptions;

	if (_sslExtension)
		ssl_ext_free(_sslExtension);

	_sslExtension = ssl_ext_new();
	ssl_ext_set_host_name(_sslExtension, server.c_str());
	ssl_ext_set_max_fragment_size(_sslExtension, 4); // 4K max size
#endif

	debug_d("connect to: %s", server.c_str());
	_canSend = false; // Wait for connection
	DnsLookup* look = new DnsLookup{this, port};
	err_t dnslook = dns_gethostbyname(server.c_str(), &addr, staticDnsResponse, look);
	if (dnslook != ERR_OK) {
		if (dnslook == ERR_INPROGRESS)
			return true;
		else {
			delete look;
			return false;
		}
	}
	delete look;

	return internalTcpConnect(addr, port);
}

bool TcpConnection::connect(IPAddress addr, uint16_t port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (!_tcp)
		initialize(tcp_new());

	_useSsl = useSsl;
#ifdef ENABLE_SSL
	_sslOptions |= sslOptions;
#endif

	return internalTcpConnect(addr, port);
}

err_t TcpConnection::onReceive(pbuf* buf)
{
	if (buf) {
		debug_d("TCP received: %d bytes", buf->tot_len);
		if (getAvailableWriteSize() > 0)
			onReadyToSendData(eTCE_Received);
	}
	else
		debug_d("TCP received: (null)");

	return ERR_OK;
}

err_t TcpConnection::onSent(uint16_t len)
{
	debug_d("TCP sent: %d", len);

	//debug_d("%d %d", tcp->state, tcp->flags); // WRONG!
	if (_tcp && getAvailableWriteSize() > 0)
		onReadyToSendData(eTCE_Sent);

	return ERR_OK;
}

err_t TcpConnection::onPoll()
{
	if (_sleep >= _timeOut && _timeOut != USHRT_MAX) {
		debug_d("TCP connection closed by timeout: %d (from %d)", _sleep, _timeOut);

		close();
		return ERR_TIMEOUT;
	}

	if (_tcp && getAvailableWriteSize() > 0) //(tcp->state >= SYN_SENT && tcp->state <= ESTABLISHED))
		onReadyToSendData(eTCE_Poll);

	return ERR_OK;
}

err_t TcpConnection::onConnected(err_t err)
{
	if (err != ERR_OK)
		debug_d("TCP connected error status: %d", err);
	else
		debug_d("TCP connected");

	_canSend = true;
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
	if (sourceEvent != eTCE_Poll)
		debug_d("onReadyToSendData: %d", sourceEvent);
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
	if (_ssl) {
		u16_t expected = ssl_calculate_write_length(_ssl, len);
		u16_t available = _tcp ? tcp_sndbuf(_tcp) : 0;
		//		debug_d("SSL: Expected: %d, Available: %d", expected, available);
		if (expected < 0 || available < expected) {
			return -1; // No memory
		}

		int written = axl_ssl_write(_ssl, (const uint8_t*)data, len);
		// debug_d("SSL: Write len: %d, Written: %d", len, written);
		if (written < 0) {
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
			len = available;
		}
		err = tcp_write(_tcp, data, len, apiflags);

#ifdef ENABLE_SSL
	}
#endif

	return (err == ERR_OK) ? len : -1;
}

int TcpConnection::write(IDataSourceStream* stream)
{
#ifdef ENABLE_SSL
	if (_ssl && !_sslConnected) {
		// wait until the SSL handshake is done.
		return 0;
	}
#endif

	// Send data from DataStream
	bool repeat;
	bool space;
	int total = 0;
	char buffer[NETWORK_SEND_BUFFER_SIZE];

	do {
		space = (tcp_sndqueuelen(_tcp) < TCP_SND_QUEUELEN);
		if (!space) {
			debug_d("WAIT FOR FREE SPACE");
			flush();
			break; // don't try to send buffers if no free space available
		}

		// Join small fragments
		int pushCount = 0;
		do {
			pushCount++;
			auto read = std::min((unsigned)NETWORK_SEND_BUFFER_SIZE, (unsigned)getAvailableWriteSize());
			int available = 0;
			if (read > 0)
				available = stream->readMemoryBlock(buffer, read);

			if (available > 0) {
				int written = write(buffer, available, TCP_WRITE_FLAG_COPY | TCP_WRITE_FLAG_MORE);
				if (written > 0) {
					total += written;
					stream->seek(written);
				}
				debug_d("Written: %d, Available: %d, isFinished: %d, PushCount: %d [TcpBuf: %d]", written, available,
						stream->isFinished(), pushCount, tcp_sndbuf(_tcp));
				repeat = written == available && !stream->isFinished() && pushCount < 25;
			}
			else
				repeat = false;
		} while (repeat);

		space = (tcp_sndqueuelen(_tcp) < TCP_SND_QUEUELEN); // && tcp_sndbuf(tcp) >= FILE_STREAM_BUFFER_SIZE;
	} while (repeat && space);

	flush();
	return total;
}

void TcpConnection::close()
{
#ifdef ENABLE_SSL
	closeSsl();
#endif

	if (!_tcp)
		return;
	debug_d("TCP connection closing");

#ifdef ENABLE_SSL
	axl_free(_tcp);
#endif

	tcp_poll(_tcp, staticOnPoll, 1);
	tcp_arg(_tcp, nullptr); // reset pointer to close connection on next callback
	_tcp = nullptr;

	checkSelfFree();
}

void TcpConnection::initialize(tcp_pcb* pcb)
{
	_tcp = pcb;
	_sleep = 0;
	_canSend = true;
#ifdef ENABLE_SSL
	axl_init(10);
#endif

	tcp_nagle_disable(_tcp);
	tcp_arg(_tcp, (void*)this);
	tcp_sent(_tcp, staticOnSent);
	tcp_recv(_tcp, staticOnReceive);
	tcp_err(_tcp, staticOnError);
	tcp_poll(_tcp, staticOnPoll, 4);

#ifdef NETWORK_DEBUG
	debug_d("+TCP connection");
#endif
}

void TcpConnection::closeTcpConnection(tcp_pcb* tpcb)
{
	if (!tpcb)
		return;

	debug_d("-TCP connection");

	tcp_arg(tpcb, nullptr);
	tcp_sent(tpcb, nullptr);
	tcp_recv(tpcb, nullptr);
	tcp_err(tpcb, nullptr);
	tcp_poll(tpcb, nullptr, 0);
	tcp_accept(tpcb, nullptr);

	auto err = tcp_close(tpcb);
	if (err != ERR_OK) {
		debug_d("tcp wait close connection");
		/* error closing, try again later in poll */
		tcp_poll(tpcb, staticOnPoll, 4);
	}
}

void TcpConnection::flush()
{
	if (_tcp && _tcp->state == ESTABLISHED) {
		//debug_d("TCP flush()");
		tcp_output(_tcp);
	}
}

bool TcpConnection::internalTcpConnect(IPAddress addr, uint16_t port)
{
	NetUtils::FixNetworkRouting();
	err_t res = tcp_connect(_tcp, addr, port, staticOnConnected);
	debug_d("TcpConnection::connect result:, %d", res);
	return res == ERR_OK;
}

err_t TcpConnection::staticOnConnected(void* arg, tcp_pcb* tcp, err_t err)
{
	TcpConnection* con = (TcpConnection*)arg;
	if (!con) {
		debug_d("OnConnected ABORT");
		//closeTcpConnection(tcp);
		tcp_abort(tcp);
		return ERR_ABRT;
	}

	return con->_onConnected(tcp, err);
}

err_t TcpConnection::_onConnected(tcp_pcb* tcp, err_t err)
{
	debug_d("OnConnected");

#ifndef ENABLE_SSL
	if (_useSsl)
		debug_w("WARNING: SSL is not compiled. Make sure to compile Sming with 'make ENABLE_SSL=1' ");
#else
	debug_d("_onConnected: useSSL = %d, Error = %d", _useSsl, err);

	if (_useSsl && err == ERR_OK) {
		int clientfd = axl_append(tcp);
		if (clientfd < 0) {
			debug_d("SSL: Unable to add LWIP tcp -> clientfd mapping");
			return ERR_OK;
		}
		else {
			uint32_t sslOptions = _sslOptions;
#ifdef SSL_DEBUG
			_sslOptions |= SSL_DISPLAY_STATES | SSL_DISPLAY_BYTES | SSL_DISPLAY_CERTS;
			debug_d("SSL: Show debug data ...");
#endif
			debug_d("SSL: Starting connection...");
#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching to 160 MHz");
			System.setCpuFrequency(eCF_160MHz); // For shorter waiting time, more power consumption.
#endif
			debug_d("SSL: handshake start (%d ms)", millis());

			if (_sslContext)
				ssl_ctx_free(_sslContext);

			_sslContext = ssl_ctx_new(SSL_CONNECT_IN_PARTS | sslOptions, 1);

			if (_sslKeyCert.keyLength && _sslKeyCert.certificateLength) {
				// if we have client certificate -> try to use it.
				if (ssl_obj_memory_load(_sslContext, SSL_OBJ_RSA_KEY, _sslKeyCert.key, _sslKeyCert.keyLength,
										_sslKeyCert.keyPassword) != SSL_OK) {
					debug_d("SSL: Unable to load client private key");
				}
				else if (ssl_obj_memory_load(_sslContext, SSL_OBJ_X509_CERT, _sslKeyCert.certificate,
											 _sslKeyCert.certificateLength, nullptr) != SSL_OK) {
					debug_d("SSL: Unable to load client certificate");
				}

				if (_freeKeyCert) {
					freeSslKeyCert();
				}
			}

			debug_d("SSL: Session Id Length: %d", _sslSessionId ? _sslSessionId->length : 0);
			if (_sslSessionId && _sslSessionId->length > 0) {
				debug_d("-----BEGIN SSL SESSION PARAMETERS-----");
				debug_hex(DBG, "Session", _sslSessionId->value, _sslSessionId->length);
				debug_d("\n-----END SSL SESSION PARAMETERS-----");
			}

			_ssl = ssl_client_new(_sslContext, clientfd, _sslSessionId ? _sslSessionId->value : nullptr,
								  _sslSessionId ? _sslSessionId->length : 0, _sslExtension);
			if (ssl_handshake_status(_ssl) != SSL_OK) {
				debug_d("SSL: handshake is in progress...");
				return SSL_OK;
			}

#ifndef SSL_SLOW_CONNECT
			debug_d("SSL: Switching back 80 MHz");
			System.setCpuFrequency(eCF_80MHz);
#endif
			if (_sslSessionId) {
				if (!_sslSessionId->value)
					_sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
				memcpy((void*)_sslSessionId->value, (void*)_ssl->session_id, _ssl->sess_id_size);
				_sslSessionId->length = _ssl->sess_id_size;
			}
		}
	}
#endif

	err_t res = onConnected(err);
	checkSelfFree();
	//debug_d("<_onConnected");
	return res;
}

err_t TcpConnection::staticOnReceive(void* arg, tcp_pcb* tcp, pbuf* p, err_t err)
{
	//Serial.println("echo_recv!");

	auto con = reinterpret_cast<TcpConnection*>(arg);
	if (!con) {
		if (p) {
			/* Inform TCP that we have taken the data. */
			tcp_recved(tcp, p->tot_len);
			pbuf_free(p);
		}
		closeTcpConnection(tcp);
		return ERR_OK;
	}

	return con->_onReceive(tcp, p, err);
}

err_t TcpConnection::_onReceive(tcp_pcb* tcp, pbuf* p, err_t err)
{
	_sleep = 0;

	if (err != ERR_OK /*&& err != ERR_CLSD && err != ERR_RST*/) {
		debug_d("Received ERROR %d", err);
		/* exit and free resources, for unknown reason */
		if (p) {
			/* Inform TCP that we have taken the data. */
			tcp_recved(tcp, p->tot_len);
			pbuf_free(p);
		}
		closeTcpConnection(tcp); // ??
		_tcp = nullptr;
		onError(err);
		//close();
		return (err == ERR_ABRT) ? ERR_ABRT : ERR_OK;
	}

	//if (tcp && tcp->state == ESTABLISHED) // If active
	/* We have taken the data. */
	if (p)
		tcp_recved(tcp, p->tot_len);
	else
		debug_d("TcpConnection::staticOnReceive: pbuf is nullptr");

#ifdef ENABLE_SSL
	if (_ssl && p) {
		WDT.alive(); /* SSL handshake needs time. In theory we have max 8 seconds before the hardware watchdog resets the device */
		struct pbuf* pout;

		int read_bytes = axl_ssl_read(_ssl, tcp, p, &pout);

		// free the SSL pbuf and put the decrypted data in the brand new pout pbuf
		if (p)
			pbuf_free(p);

		if (read_bytes < SSL_OK) {
			debug_d("SSL: Got error: %d", read_bytes);
			if (read_bytes == SSL_CLOSE_NOTIFY)
				return ERR_OK;

			close();
			closeTcpConnection(tcp);
			return read_bytes;
		}

		if (read_bytes == 0) {
			if (!_sslConnected && ssl_handshake_status(_ssl) == SSL_OK) {
				_sslConnected = true;
				debug_d("SSL: Handshake done (%d ms).", millis());
#ifndef SSL_SLOW_CONNECT
				debug_d("SSL: Switching back to 80 MHz");
				System.setCpuFrequency(eCF_80MHz); // Preserve some CPU cycles
#endif
				if (onSslConnected(_ssl) != ERR_OK) {
					close();
					closeTcpConnection(tcp);

					return ERR_ABRT;
				}

				if (_sslSessionId) {
					if (!_sslSessionId->value)
						_sslSessionId->value = new uint8_t[SSL_SESSION_ID_SIZE];
					memcpy((void*)_sslSessionId->value, (void*)_ssl->session_id, _ssl->sess_id_size);
					_sslSessionId->length = _ssl->sess_id_size;
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

	if (p)
		pbuf_free(p);
	else {
		close();
		closeTcpConnection(tcp);
	}

	checkSelfFree();
	//debug_d("<_onReceive");
	return res;
}

err_t TcpConnection::staticOnSent(void* arg, tcp_pcb* tcp, uint16_t len)
{
	auto con = (TcpConnection*)arg;
	return con ? con->_onSent(tcp, len) : ERR_OK;
}

err_t TcpConnection::_onSent(tcp_pcb* tcp, uint16_t len)
{
	_sleep = 0;
	err_t res = onSent(len);
	checkSelfFree();
	//debug_d("<_onSent");
	return res;
}

err_t TcpConnection::staticOnPoll(void* arg, tcp_pcb* tcp)
{
	TcpConnection* con = (TcpConnection*)arg;

	if (!con) {
		closeTcpConnection(tcp);
		return ERR_OK;
	}

	return con->_onPoll(tcp);
}

err_t TcpConnection::_onPoll(tcp_pcb* tcp)
{
	//if (tcp->state != ESTABLISHED)
	//	return ERR_OK;

	_sleep++;
	err_t res = onPoll();
	checkSelfFree();
	//debug_d("<staticOnPoll");
	return res;
}

void TcpConnection::staticOnError(void* arg, err_t err)
{
	auto con = (TcpConnection*)arg;
	if (con)
		con->_onError(err);
}

void TcpConnection::_onError(err_t err)
{
	// IMPORTANT. No available connection after error!
	_tcp = nullptr;

	onError(err);
	checkSelfFree();
	//debug_d("<staticOnError");
}

void TcpConnection::staticDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, void* arg)
{
	DnsLookup* dlook = (DnsLookup*)arg;
	if (!dlook)
		return;

	if (ipaddr) {
		IPAddress ip = *ipaddr;
		debug_d("DNS record found: %s = %d.%d.%d.%d", name, ip[0], ip[1], ip[2], ip[3]);

		dlook->con->internalTcpConnect(ip, dlook->port);
	}
	else {
#ifdef NETWORK_DEBUG
		debug_d("DNS record _not_ found: %s", name);
#endif

		closeTcpConnection(dlook->con->_tcp);
		dlook->con->_tcp = nullptr;
		dlook->con->close();
	}

	delete dlook;
}

#ifdef ENABLE_SSL

void TcpConnection::addSslOptions(uint32_t sslOptions)
{
	_sslOptions |= sslOptions;
}

bool TcpConnection::setSslKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
								  const char* keyPassword /* = nullptr */, bool freeAfterHandshake /* = false */)
{
	delete[] _sslKeyCert.key;
	delete[] _sslKeyCert.certificate;
	delete[] _sslKeyCert.keyPassword;
	_sslKeyCert.keyPassword = nullptr;

	_sslKeyCert.key = new uint8_t[keyLength];
	_sslKeyCert.certificate = new uint8_t[certificateLength];
	int passwordLength = 0;
	if (keyPassword) {
		passwordLength = strlen(keyPassword);
		_sslKeyCert.keyPassword = new char[passwordLength + 1];
	}

	if (!(_sslKeyCert.key && _sslKeyCert.certificate && (passwordLength == 0 || _sslKeyCert.keyPassword))) {
		return false;
	}

	memcpy(_sslKeyCert.key, key, keyLength);
	memcpy(_sslKeyCert.certificate, certificate, certificateLength);
	if (keyPassword)
		memcpy(_sslKeyCert.keyPassword, keyPassword, passwordLength);
	_freeKeyCert = freeAfterHandshake;

	_sslKeyCert.keyLength = keyLength;
	_sslKeyCert.certificateLength = certificateLength;
	_sslKeyCert.keyLength = keyLength;

	return true;
}

bool TcpConnection::setSslKeyCert(const SSLKeyCertPair& keyCertPair, bool freeAfterHandshake /* = false */)
{
	freeSslKeyCert();
	_sslKeyCert = keyCertPair;
	_freeKeyCert = freeAfterHandshake;

	return true;
}

void TcpConnection::closeSsl()
{
	if (!_ssl)
		return;

	debug_d("SSL: closing ...");
	ssl_ctx_free(_sslContext);
	_sslContext = nullptr;
	_sslExtension = nullptr;
	_ssl = nullptr;
	_sslConnected = false;
}

#endif
