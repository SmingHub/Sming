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
#include "../../axtls-8266/compat/lwipr_compat.h"
#include "../Clock.h"
#endif

#include "../Wiring/WiringFrameworkDependencies.h"
#include "IPAddress.h"
#include "../Delegate.h"

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

typedef struct {
	uint8_t* certSha1 = NULL; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = NULL; // << public key SHA256 fingerprint
} SSLFingerprints;

typedef struct {
	uint8_t* key = NULL;
	int keyLength = 0;
	char* keyPassword = NULL;
	uint8_t* certificate = NULL;
	int certificateLength = 0;
} SSLKeyCertPair;

typedef struct {
	uint8_t* value = NULL;
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

class TcpConnection
{
	friend class TcpServer;

public:
	TcpConnection(bool autoDestruct);
	TcpConnection(tcp_pcb* connection, bool autoDestruct);
	virtual ~TcpConnection();

public:
	virtual bool connect(const String& server, int port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual void close();

	// return -1 on error
	int writeString(const char* data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	int writeString(const String& data, uint8_t apiflags = TCP_WRITE_FLAG_COPY);
	// return -1 on error
	virtual int write(const char* data, int len,
					  uint8_t apiflags = TCP_WRITE_FLAG_COPY); // flags: TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE
	int write(IDataSourceStream* stream);
	__forceinline uint16_t getAvailableWriteSize()
	{
		return (canSend && tcp) ? tcp_sndbuf(tcp) : 0;
	}
	void flush();

	void setTimeOut(uint16_t waitTimeOut);
	IPAddress getRemoteIp()
	{
		return (tcp == NULL) ? INADDR_NONE : IPAddress(tcp->remote_ip);
	};
	uint16_t getRemotePort()
	{
		return (tcp == NULL) ? 0 : tcp->remote_port;
	};

	/**
	 * @brief Sets a callback to be called when the object instance is destroyed
	 * @param TcpServerConnectionDestroyedDelegate destroyedDelegate - callback
	 */
	void setDestroyedDelegate(TcpConnectionDestroyedDelegate destroyedDelegate);

#ifdef ENABLE_SSL
	void addSslOptions(uint32_t sslOptions);

	// start deprecated
	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @deprecated: Use setSslKeyCert instead
	 *
	 * @note  This method makes copy of the data.
	 *
	 * @param const uint8_t *keyData
	 * @param int keyLength
	 * @param const uint8_t *certificateData
	 * @param int certificateLength
	 * @param const char *keyPassword
	 * @param bool freeAfterHandshake
	 *
	 * @return bool  true of success, false or failure
	 */
	bool setSslClientKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
							 const char* keyPassword = NULL, bool freeAfterHandshake = false)
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
	 * @param const uint8_t *keyData
	 * @param int keyLength
	 * @param const uint8_t *certificateData
	 * @param int certificateLength
	 * @param const char *keyPassword
	 * @param bool freeAfterHandshake
	 *
	 * @return bool  true of success, false or failure
	 */
	bool setSslKeyCert(const uint8_t* key, int keyLength, const uint8_t* certificate, int certificateLength,
					   const char* keyPassword = NULL, bool freeAfterHandshake = false);

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
	void freeSslKeyCert();

	SSL* getSsl();
#endif

protected:
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

	static err_t staticOnConnected(void* arg, tcp_pcb* tcp, err_t err);
	static err_t staticOnReceive(void* arg, tcp_pcb* tcp, pbuf* p, err_t err);
	static err_t staticOnSent(void* arg, tcp_pcb* tcp, uint16_t len);
	static err_t staticOnPoll(void* arg, tcp_pcb* tcp);
	static void staticOnError(void* arg, err_t err);
	static void staticDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, void* arg);

	static void closeTcpConnection(tcp_pcb* tpcb);
	void initialize(tcp_pcb* pcb);

private:
	inline void checkSelfFree()
	{
		if(tcp == NULL && autoSelfDestruct)
			delete this;
	}

protected:
	tcp_pcb* tcp = NULL;
	uint16_t sleep;
	uint16_t timeOut = USHRT_MAX; // << By default a TCP connection does not have a time out
	bool canSend;
	bool autoSelfDestruct;
#ifdef ENABLE_SSL
	SSL* ssl = nullptr;
	SSLCTX* sslContext = nullptr;
	SSL_EXTENSIONS* sslExtension = nullptr;
	bool sslConnected = false;
	uint32_t sslOptions = 0;
	SSLKeyCertPair sslKeyCert;
	bool freeKeyCert = false;
	SSLSessionId* sslSessionId = NULL;
#endif
	bool useSsl = false;

private:
	TcpConnectionDestroyedDelegate destroyedDelegate = 0;

#ifdef ENABLE_SSL
	void closeSsl();
#endif
};

/** @} */
#endif /* _SMING_CORE_TCPCONNECTION_H_ */
