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
#include "../Clock.h"
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

#ifdef ENABLE_SSL
enum SslFingerprintType {
	eSFT_CertSha1 = 0, // << Fingerprint based on the SHA1 value of the certificate.
					  //     Every time a certificate is renewed this value will change.
	eSFT_PkSha256,  // << Fingerprint based on the SHA256 value of the public key subject in the certificate.
					//    Only when the private key used to generate the certificate is used then that fingerprint
};

typedef struct {
	uint8_t* certSha1 = NULL; // << certificate SHA1 fingerprint
	uint8_t* pkSha256 = NULL; // << public key SHA256 fingerprint
} SSLFingerprints;

typedef struct {
	uint8_t *key = NULL;
	int keyLength = 0;
	char *keyPassword = NULL;
	uint8_t *certificate = NULL;
	int certificateLength = 0;
} SSLKeyCertPair;

typedef struct {
	uint8_t *value = NULL;
	int length = 0;
} SSLSessionId;

#endif

struct pbuf;
class String;
class IDataSourceStream;
class IPAddress;
class TcpServer;

class TcpConnection
{
	friend class TcpServer;

public:
	TcpConnection(bool autoDestruct);
	TcpConnection(tcp_pcb* connection, bool autoDestruct);
	virtual ~TcpConnection();

public:
	virtual bool connect(String server, int port, bool useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, bool useSsl = false, uint32_t sslOptions = 0);
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

#ifdef ENABLE_SSL
	void addSslOptions(uint32_t sslOptions);

	/**
	 * @brief Sets the SHA1 certificate finger print.
	 * 		  The latter will be used after successful handshake to check against the fingerprint of the other side.
	 *
	 * @deprecated This method will be removed in future releases. Use pinCertificate instead.
	 *
	 * @param const uint8_t *data
	 * @param int length
	 * @return bool  true of success, false or failure
	 */
	__forceinline bool setSslFingerprint(const uint8_t *data, int length = SHA1_SIZE) {
		return pinCertificate(data, eSFT_CertSha1);
	}

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 * 			Check if SHA256 hash of Subject Public Key Info matches the one given.
	 * @note    For HTTP public key pinning (RFC7469), the SHA-256 hash of the
	 * 		    Subject Public Key Info (which usually only changes when the public key changes)
	 * 		    is used rather than the SHA-1 hash of the entire certificate
	 * 		    (which will change on each certificate renewal).
	 * @param const uint8_t *finterprint - the fingeprint data agains which the match should be perfomed
	 * @param SslFingerprintType type - the fingerprint type
	 * @note    Type: eSFT_PkSha256
	 * 			For HTTP public key pinning (RFC7469), the SHA-256 hash of the
	 * 		    Subject Public Key Info (which usually only changes when the public key changes)
	 * 		    is used rather than the SHA-1 hash of the entire certificate
	 * 		    (which will change on each certificate renewal).
	 * 		    Advantages: The
	 * 		    Disadvantages: Takes more time (in ms) to verify.
	 * @note    Type: eSFT_CertSha1
	 * 			The SHA1 hash of the remote certificate will be calculated and compared with the given one.
	 * 			Disadvantages: The hash needs to be updated every time the remote server updates its certificate
	 * @return bool  true of success, false or failure
	 */
	bool pinCertificate(const uint8_t *fingerprint, SslFingerprintType type, bool freeAfterHandshake = false);

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 *
	 * @note  The data inside the fingerprints parameter is passed by reference
	 *
	 * @param SSLFingerprints - passes the certificate fingerprints by reference.
	 *
	 * @return bool  true of success, false or failure
	 */
	bool pinCertificate(SSLFingerprints fingerprints, bool freeAfterHandshake = false);

	/**
	 * @brief Sets client private key, certificate and password from memory
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
	bool setSslClientKeyCert(const uint8_t *key, int keyLength,
							 const uint8_t *certificate, int certificateLength,
							 const char *keyPassword = NULL, bool freeAfterHandshake = false);

	/**
	* @brief Sets client private key, certificate and password from memory
	*
	* @note  This method passes the certificate key chain by reference
	*
	* @param SSLKeyCertPair
	* @param bool freeAfterHandshake
	*
	* @return bool  true of success, false or failure
	*/
	bool setSslClientKeyCert(SSLKeyCertPair clientKeyCert, bool freeAfterHandshake = false);

	/**
	 * @brief Frees the memory used for the client key and certificate pair
	 */
	void freeSslClientKeyCert();

	/**
	 * @brief Frees the memory used for SSL fingerprinting
	 */
	void freeSslFingerprints();

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
	tcp_pcb *tcp = NULL;
	uint16_t sleep;
	uint16_t timeOut;
	bool canSend;
	bool autoSelfDestruct;
#ifdef ENABLE_SSL
	SSL *ssl = nullptr;
	SSLCTX *sslContext = nullptr;
	SSL_EXTENSIONS *ssl_ext=NULL;
	SSLFingerprints sslFingerprint;
	bool sslConnected = false;
	uint32_t sslOptions=0;
	SSLKeyCertPair clientKeyCert;
	bool freeClientKeyCert = false;
	bool freeFingerprints = false;
	SSLSessionId* sslSessionId = NULL;
#endif
	bool useSsl = false;
};

#endif /* _SMING_CORE_TCPCONNECTION_H_ */
