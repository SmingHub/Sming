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

typedef struct {
	uint8_t* certSha1 = nullptr; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = nullptr; // << public key SHA256 fingerprint

	void free()
	{
		delete certSha1;
		certSha1 = nullptr;
		delete pkSha256;
		pkSha256 = nullptr;
	}
} SSLFingerprints;

/** @brief Structure to manage an SSL key certificate with optional password
 *  @note Do not set member variables directly, use provided methods
 */
struct SSLKeyCertPair {
	uint8_t* key = nullptr;
	unsigned keyLength = 0;
	char* keyPassword = nullptr;
	uint8_t* certificate = nullptr;
	unsigned certificateLength = 0;

	~SSLKeyCertPair()
	{
		free();
	}

	bool isValid()
	{
		return keyLength != 0 && certificateLength != 0;
	}

	/** @brief Create certificate using provided values
	 *  @param newKey
	 *  @param newKeyLength
	 *  @param newCertificate
	 *  @param newCertificateLength
	 *  @param newKeyPassword
	 *  @retval bool false on memory allocation failure
	 *  @note We take a new copy of the certificate
	 */
	bool assign(const uint8_t* newKey, unsigned newKeyLength, const uint8_t* newCertificate,
				unsigned newCertificateLength, const char* newKeyPassword = nullptr)
	{
		free();

		key = new uint8_t[newKeyLength];
		if(key == nullptr) {
			return false;
		}
		memcpy(key, newKey, newKeyLength);
		keyLength = newKeyLength;

		certificate = new uint8_t[newCertificateLength];
		if(certificate == nullptr) {
			return false;
		}
		memcpy(certificate, newCertificate, newCertificateLength);
		certificateLength = newCertificateLength;

		unsigned passwordLength = (newKeyPassword == nullptr) ? 0 : strlen(newKeyPassword);
		if(passwordLength > 0) {
			keyPassword = new char[passwordLength + 1];
			if(keyPassword == nullptr) {
				return false;
			}
			memcpy(keyPassword, newKeyPassword, passwordLength);
			keyPassword[passwordLength] = '\0';
		}

		return true;
	}

	/** @brief Assign another certificate to this structure
	 *  @param keyCert
	 *  @retval bool false on memory allocation failure
	 *  @note We take a new copy of the certificate
	 */
	bool assign(const SSLKeyCertPair& keyCert)
	{
		return assign(keyCert.key, keyCert.keyLength, keyCert.certificate, keyCert.certificateLength,
					  keyCert.keyPassword);
	}

	SSLKeyCertPair& operator=(const SSLKeyCertPair& keyCert)
	{
		assign(keyCert);
		return *this;
	}

	void free()
	{
		delete[] key;
		key = nullptr;

		delete[] certificate;
		certificate = nullptr;

		delete[] keyPassword;
		keyPassword = nullptr;

		keyLength = 0;
		certificateLength = 0;
	}
};

typedef struct {
	uint8_t* value = nullptr;
	int length = 0;
} SSLSessionId;

#endif

struct pbuf;
class String;
class IDataSourceStream;
class IPAddress;
class TcpConnection;

typedef Delegate<void(TcpConnection&)> TcpConnectionDestroyedDelegate;

class TcpConnection
{
public:
	TcpConnection(bool autoDestruct) : autoSelfDestruct(autoDestruct)
	{
	}

	TcpConnection(tcp_pcb* connection, bool autoDestruct) : autoSelfDestruct(autoDestruct)
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

	// return -1 on error
	int writeString(const String& data, uint8_t apiflags = TCP_WRITE_FLAG_COPY)
	{
		return write(data.c_str(), data.length(), apiflags);
	}

	/** @brief Base write operation
	 *  @param data
	 *  @param len
	 *  @param apiflags TCP_WRITE_FLAG_COPY, TCP_WRITE_FLAG_MORE
	 *	@retval int -1 on error
	 */
	virtual int write(const char* data, int len, uint8_t apiflags = TCP_WRITE_FLAG_COPY);

	int write(IDataSourceStream* stream);

	__forceinline uint16_t getAvailableWriteSize()
	{
		return (canSend && tcp) ? tcp_sndbuf(tcp) : 0;
	}

	void flush();

	void setTimeOut(uint16_t waitTimeOut);

	IPAddress getRemoteIp() const
	{
		return (tcp == nullptr) ? INADDR_NONE : IPAddress(tcp->remote_ip);
	}

	uint16_t getRemotePort() const
	{
		return (tcp == nullptr) ? 0 : tcp->remote_port;
	}

	/**
	 * @brief Sets a callback to be called when the object instance is destroyed
	 * @param TcpServerConnectionDestroyedDelegate destroyedDelegate - callback
	 */
	void setDestroyedDelegate(TcpConnectionDestroyedDelegate destroyedDelegate)
	{
		this->destroyedDelegate = destroyedDelegate;
	}

#ifdef ENABLE_SSL
	void addSslOptions(uint32_t sslOptions)
	{
		this->sslOptions |= sslOptions;
	}

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
					   const char* keyPassword = nullptr, bool freeAfterHandshake = false)
	{
		freeKeyCertAfterHandshake = freeAfterHandshake;
		return sslKeyCert.assign(key, keyLength, certificate, certificateLength, keyPassword);
	}

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
	bool setSslKeyCert(const SSLKeyCertPair& keyCert, bool freeAfterHandshake = false)
	{
		freeKeyCertAfterHandshake = freeAfterHandshake;
		return sslKeyCert.assign(keyCert);
	}

	/**
	 * @brief Frees the memory used for the key and certificate pair
	 */
	void freeSslKeyCert()
	{
		sslKeyCert.free();
	}

	// Called by TcpServer
	void setSsl(SSL* ssl)
	{
		this->ssl = ssl;
		useSsl = true;
	}

	SSL* getSsl()
	{
		return ssl;
	}

#endif

protected:
	void initialize(tcp_pcb* pcb);
	bool internalConnect(IPAddress addr, uint16_t port);

	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual err_t onPoll();
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
#ifdef ENABLE_SSL
	virtual err_t onSslConnected(SSL* ssl);
#endif

	// These methods are called via LWIP handlers
	err_t internalOnConnected(err_t err);
	err_t internalOnReceive(pbuf* p, err_t err);
	err_t internalOnSent(uint16_t len);
	err_t internalOnPoll();
	void internalOnError(err_t err);
	void internalOnDnsResponse(const char* name, LWIP_IP_ADDR_T* ipaddr, int port);

private:
	static err_t staticOnPoll(void* arg, tcp_pcb* tcp);
	static void closeTcpConnection(tcp_pcb* tpcb);

	inline void checkSelfFree()
	{
		if(tcp == nullptr && autoSelfDestruct) {
			delete this;
		}
	}

protected:
	tcp_pcb* tcp = nullptr;
	uint16_t sleep = 0;
	uint16_t timeOut = USHRT_MAX; // << By default a TCP connection does not have a time out
	bool canSend = true;
	bool autoSelfDestruct = true;
#ifdef ENABLE_SSL
	SSL* ssl = nullptr;
	SSLCTX* sslContext = nullptr;
	SSL_EXTENSIONS* sslExtension = nullptr;
	bool sslConnected = false;
	uint32_t sslOptions = 0;
	SSLKeyCertPair sslKeyCert;
	bool freeKeyCertAfterHandshake = false;
	SSLSessionId* sslSessionId = nullptr;
#endif
	bool useSsl = false;

private:
	TcpConnectionDestroyedDelegate destroyedDelegate = nullptr;

#ifdef ENABLE_SSL
	void closeSsl();
#endif
};

/** @} */
#endif /* _SMING_CORE_TCPCONNECTION_H_ */
