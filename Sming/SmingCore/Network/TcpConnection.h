/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_TCPCONNECTION_H_
#define _SMING_CORE_TCPCONNECTION_H_

#ifdef ENABLE_SSL
#include "../../axtls-8266/compat/lwipr_compat.h"
#endif

#include "../Wiring/WiringFrameworkDependencies.h"
#include "IPAddress.h"


#define NETWORK_DEBUG

#define NETWORK_SEND_BUFFER_SIZE 1024

enum TcpConnectionEvent
{
	// Occurs after connection establishment
	eTCE_Connected = 0,
	// Occurs on data receive
	eTCE_Received,
	// Occurs when previous sending was completed
	eTCE_Sent,
	// Occurs on waiting
	eTCE_Poll
};

struct pbuf;
class String;
class IDataSourceStream;
class IPAddress;

typedef struct {
	uint8_t *key = NULL;
	int keyLength = 0;
	char *keyPassword = NULL;
	uint8_t *certificate = NULL;
	int certificateLength = 0;
} SSLKeyCertPair;

class TcpConnection
{
public:
	TcpConnection(bool autoDestruct);
	TcpConnection(tcp_pcb* connection, bool autoDestruct);
	virtual ~TcpConnection();

public:
	virtual bool connect(String server, int port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual void close();

	// return -1 on error
	int writeString(const char* data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	int writeString(const String data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	// return -1 on error
	virtual int write(const char* data, int len, uint8_t apiflags = TCP_WRITE_FLAG_COPY); // flags: TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE
	int write(IDataSourceStream* stream);
	__forceinline uint16_t getAvailableWriteSize() { return (canSend && tcp) ? tcp_sndbuf(tcp) : 0; }
	void flush();

	void setTimeOut(uint16_t waitTimeOut);
	IPAddress getRemoteIp()  { return (tcp == NULL) ? INADDR_NONE : IPAddress(tcp->remote_ip);};
	uint16_t getRemotePort() { return (tcp == NULL) ? 0 : tcp->remote_port; };

	void addSslOptions(uint32_t sslOptions);

	/**
	 * @brief Sets the SHA1 certificate finger print.
	 * 		  The latter will be used after successful handshake to check against the fingerprint of the other side.
	 * @param const uint8_t *data
	 * @param int length
	 * @return boolean  true of success, false or failure
	 */
	boolean setSslFingerprint(const uint8_t *data, int length = 20);

	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @param const uint8_t *keyData
	 * @param int keyLength
	 * @param const uint8_t *certificateData
	 * @param int certificateLength
	 * @param const char *keyPassword
	 * @param boolean freeAfterHandshake
	 *
	 * @return boolean  true of success, false or failure
	 */
	boolean setSslClientKeyCert(const uint8_t *key, int keyLength,
							 const uint8_t *certificate, int certificateLength,
							 const char *keyPassword = NULL, boolean freeAfterHandshake = false);

	/**
	 * @brief Frees the memory used for the client key and certificate pair
	 */
	void freeSslClientKeyCert();

#ifdef ENABLE_SSL
	SSL* getSsl();
#endif

protected:
	bool internalTcpConnect(IPAddress addr, uint16_t port);
	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf *buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

	static err_t staticOnConnected(void *arg, tcp_pcb *tcp, err_t err);
	static err_t staticOnReceive(void *arg, tcp_pcb *tcp, pbuf *p, err_t err);
	static err_t staticOnSent(void *arg, tcp_pcb *tcp, uint16_t len);
	static err_t staticOnPoll(void *arg, tcp_pcb *tcp);
	static void staticOnError(void *arg, err_t err);
	static void staticDnsResponse(const char *name, ip_addr_t *ipaddr, void *arg);

	static void closeTcpConnection(tcp_pcb *tpcb);
	void initialize(tcp_pcb* pcb);

private:
	inline void checkSelfFree() { if (tcp == NULL && autoSelfDestruct) delete this; }

protected:
	tcp_pcb *tcp;
	uint16_t sleep;
	uint16_t timeOut;
	bool canSend;
	bool autoSelfDestruct;
#ifdef ENABLE_SSL
	SSL *ssl = nullptr;
	SSLCTX *sslContext = nullptr;
	SSL_EXTENSIONS *ssl_ext=NULL;
#endif
	boolean useSsl = false;
	uint8_t *sslFingerprint=null;
	boolean sslConnected = false;
	uint32_t sslOptions=0;
	SSLKeyCertPair clientKeyCert;
	boolean freeClientKeyCert = false;
};

#endif /* _SMING_CORE_TCPCONNECTION_H_ */
