/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup tcp TCP
 *  @ingroup networking
 *  @{
 */

#ifndef _SMING_CORE_TCPCONNECTION_H_
#define _SMING_CORE_TCPCONNECTION_H_

#ifdef ENABLE_SSL
#include "../axtls-8266/compat/lwipr_compat.h"
#include "Clock.h"
#endif

#include "WiringFrameworkDependencies.h"
#include "IPAddress.h"
#include "Delegate.h"

#define NETWORK_DEBUG

#define NETWORK_SEND_BUFFER_SIZE 1024

enum TcpConnectionEvent {
	// Occurs after connection establishment
	eTCE_Connected = 0,
	// Occurs on data receive
	eTCE_Received,
	// Occurs when previous sending was completed
	eTCE_Sent,
	// Occurs on waiting
	eTCE_Poll
};

#ifdef ENABLE_SSL
enum SslFingerprintType {
	eSFT_CertSha1 = 0, // << Fingerprint based on the SHA1 value of the certificate.
					   //     Every time a certificate is renewed this value will change.
	eSFT_PkSha256,	 // << Fingerprint based on the SHA256 value of the public key subject in the certificate.
					   //    Only when the private key used to generate the certificate is used then that fingerprint
};

typedef struct
{
	uint8_t* certSha1 = nullptr; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = nullptr; // << public key SHA256 fingerprint
} SSLFingerprints;

static inline void freeSSLFingerprints(SSLFingerprints& fp)
{
	delete fp.certSha1;
	fp.certSha1 = nullptr;
	delete fp.pkSha256;
	fp.pkSha256 = nullptr;
}

typedef struct
{
	uint8_t* key = nullptr;
	int keyLength = 0;
	char* keyPassword = nullptr;
	uint8_t* certificate = nullptr;
	int certificateLength = 0;
} SSLKeyCertPair;

static inline void freeSslKeyCert(SSLKeyCertPair& cert)
{
	if (cert.key) {
		delete[] cert.key;
		cert.key = nullptr;
	}

	if (cert.certificate) {
		delete[] cert.certificate;
		cert.certificate = nullptr;
	}

	if (cert.keyPassword) {
		delete[] cert.keyPassword;
		cert.keyPassword = nullptr;
	}

	cert.keyLength = 0;
	cert.certificateLength = 0;
}

typedef struct
{
	uint8_t* value = nullptr;
	int length = 0;
} SSLSessionId;

#endif

struct pbuf;
class String;
class IDataSourceStream;
class IPAddress;
class TcpServer;
class TcpConnection;

typedef Delegate<void(TcpConnection&)> TcpConnectionDestroyedDelegate;

class TcpConnection {
public:
	TcpConnection(bool autoDestruct) : _autoSelfDestruct(autoDestruct)
	{}

	TcpConnection(tcp_pcb* connection, bool autoDestruct) : _autoSelfDestruct(autoDestruct)
	{
		initialize(connection);
	}

	virtual ~TcpConnection();

public:
	virtual bool connect(const String& server, int port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual void close();

	// return -1 on error
	int writeString(const char* data, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		return write(data, strlen(data), apiflags);
	}

	int writeString(const String& data, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		return write(data.c_str(), data.length(), apiflags);
	}

	// return -1 on error
	virtual int write(const char* data, int len,
					  uint8_t apiflags = TCP_WRITE_FLAG_COPY); // flags: TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE

	int write(IDataSourceStream* stream);

	uint16_t getAvailableWriteSize()
	{
		return (_canSend && _tcp) ? tcp_sndbuf(_tcp) : 0;
	}

	void flush();

	void setTimeOut(uint16_t waitTimeOut)
	{
		debug_d("timeout updating: %d -> %d", _timeOut, waitTimeOut);
		_timeOut = waitTimeOut;
	}

	IPAddress getRemoteIp() const
	{
		return _tcp ? IPAddress(_tcp->remote_ip) : INADDR_NONE;
	}

	uint16_t getRemotePort() const
	{
		return _tcp ? _tcp->remote_port : 0;
	}

	/**
	 * @brief Sets a callback to be called when the object instance is destroyed
	 * @param TcpServerConnectionDestroyedDelegate destroyedDelegate - callback
	 */
	void setDestroyedDelegate(TcpConnectionDestroyedDelegate destroyedDelegate)
	{
		_destroyedDelegate = destroyedDelegate;
	}

#ifdef ENABLE_SSL
	void addSslOptions(uint32_t sslOptions);

	// start deprecated
	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @deprecated: Use setSslKeyCert instead
	 *
	 * @note  This method makes copy of the data.
	 *
	 * @param const uint8_t* keyData
	 * @param int keyLength
	 * @param const uint8_t* certificateData
	 * @param int certificateLength
	 * @param const char* keyPassword
	 * @param bool freeAfterHandshake
	 *
	 * @return bool  true of success, false or failure
	 */
	bool setSslClientKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
							 const char* keyPassword = nullptr, bool freeAfterHandshake = false)
	{
		return setSslKeyCert(key, keyLength, certificate, certificateLength, keyPassword, freeAfterHandshake);
	}

	/**
	* @brief Sets client private key, certificate and password from memory
	* @deprecated: Use setSslKeyCert instead
	*
	* @note  This method passes the certificate key chain by reference
	*
	* @param SSLKeyCertPair
	* @param bool freeAfterHandshake
	*
	* @return bool  true of success, false or failure
	*/
	bool setSslClientKeyCert(const SSLKeyCertPair& clientKeyCert, bool freeAfterHandshake = false)
	{
		return setSslKeyCert(clientKeyCert, freeAfterHandshake);
	}

	/**
	 * @brief Frees the memory used for the key and certificate pair
	 * @deprecated: Use freeSslKeyCert instead
	 */
	void freeSslClientKeyCert()
	{
		freeSslKeyCert();
	}

	// end deprecated

	/**
	 * @brief Sets private key, certificate and password from memory for the SSL connection
	 * 		  If this methods is called from a client then it sets the client key and certificate
	 * 		  If it is called from a server then it sets the server certificate and key.
	 * 		  Server and Client certificates differ. Client certificate is used for identification.
	 * 		  Server certificate is used for encrypt/decrypt the data.
	 * 		  Make sure to use the correct certificate for the desired goal.
	 *
	 * @note  This method makes copy of the data.
	 *
	 * @param const uint8_t* keyData
	 * @param int keyLength
	 * @param const uint8_t* certificateData
	 * @param int certificateLength
	 * @param const char* keyPassword
	 * @param bool freeAfterHandshake
	 *
	 * @return bool  true of success, false or failure
	 */
	bool setSslKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
					   const char* keyPassword = nullptr, bool freeAfterHandshake = false);

	/**
	* @brief Sets private key, certificate and password from memory for the SSL connection
	* 	 	 If this methods is called from a client then it sets the client key and certificate
	* 		 If it is called from a server then it sets the server certificate and key.
	* 		 Server and Client certificates differ. Client certificate is used for identification.
	* 		 Server certificate is used for encrypt/decrypt the data.
	* 		 Make sure to use the correct certificate for the desired goal.
	*
	* @note  This method passes the certificate key chain by reference
	*
	* @param SSLKeyCertPair
	* @param bool freeAfterHandshake
	*
	* @return bool  true of success, false or failure
	*/
	bool setSslKeyCert(const SSLKeyCertPair& keyCert, bool freeAfterHandshake = false);

	/**
	 * @brief Frees the memory used for the key and certificate pair
	 */
	void freeSslKeyCert()
	{
		::freeSslKeyCert(_sslKeyCert);
	}

	void setSslSessionId(SSLSessionId* sessionId)
	{
		_sslSessionId = sessionId;
	}

	// Called by TcpServer
	void setSsl(SSL* ssl)
	{
		_ssl = ssl;
		_useSsl = true;
	}

	SSL* getSsl()
	{
		return _ssl;
	}

#endif

protected:
	void initialize(tcp_pcb* pcb);
	bool internalTcpConnect(IPAddress addr, uint16_t port);

	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
#ifdef ENABLE_SSL
	virtual err_t onSslConnected(SSL* ssl);
#endif

	// These do most of the work called from static methods
	err_t _onConnected(tcp_pcb* tcp, err_t err);
	err_t _onReceive(tcp_pcb* tcp, pbuf* p, err_t err);
	err_t _onSent(tcp_pcb* tcp, uint16_t len);
	err_t _onPoll(tcp_pcb* tcp);
	void _onError(err_t err);

private:
	// Methods called directly from TCP stack
	static err_t staticOnConnected(void* arg, tcp_pcb* tcp, err_t err);
	static err_t staticOnReceive(void* arg, tcp_pcb* tcp, pbuf* p, err_t err);
	static err_t staticOnSent(void* arg, tcp_pcb* tcp, uint16_t len);
	static err_t staticOnPoll(void* arg, tcp_pcb* tcp);
	static void staticOnError(void* arg, err_t err);

	static void staticDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, void* arg);

	static void closeTcpConnection(tcp_pcb* tpcb);

private:
	inline void checkSelfFree()
	{
		if (!_tcp && _autoSelfDestruct)
			delete this;
	}

protected:
	tcp_pcb* _tcp = nullptr;
	uint16_t _sleep = 0;
	uint16_t _timeOut = 70;
	bool _canSend = true;
	bool _autoSelfDestruct = true;
#ifdef ENABLE_SSL
	SSL* _ssl = nullptr;
	SSLCTX* _sslContext = nullptr;
	SSL_EXTENSIONS* _sslExtension = nullptr;
	bool _sslConnected = false;
	uint32_t _sslOptions = 0;
	SSLKeyCertPair _sslKeyCert;
	bool _freeKeyCert = false;
	SSLSessionId* _sslSessionId = nullptr;
#endif
	bool _useSsl = false;

private:
	TcpConnectionDestroyedDelegate _destroyedDelegate = nullptr;

#ifdef ENABLE_SSL
	void closeSsl();
#endif
};

/** @} */
#endif /* _SMING_CORE_TCPCONNECTION_H_ */
