/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/** @defgroup   tcpclient Clients
 *  @brief      Provides base TCP client
 *  @ingroup    tcp
 *  @{
 */

#ifndef _SMING_CORE_TCPCLIENT_H_
#define _SMING_CORE_TCPCLIENT_H_

#include "TcpConnection.h"
#include "Delegate.h"

#ifdef ENABLE_SSL
#include "SslValidator.h"
#endif

class TcpClient;
class ReadWriteStream;
class IPAddress;

//typedef void (*TcpClientEventDelegate)(TcpClient& client, TcpConnectionEvent sourceEvent);
//typedef void (*TcpClientBoolDelegate)(TcpClient& client, bool successful);
//typedef bool (*TcpClientDataDelegate)(TcpClient& client, char *data, int size);

typedef Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)> TcpClientEventDelegate;
typedef Delegate<void(TcpClient& client, bool successful)> TcpClientCompleteDelegate;
typedef Delegate<bool(TcpClient& client, char* data, int size)> TcpClientDataDelegate;

enum TcpClientState { eTCS_Ready, eTCS_Connecting, eTCS_Connected, eTCS_Successful, eTCS_Failed };

class TcpClient : public TcpConnection
{
public:
	TcpClient(bool autoDestruct);
	TcpClient(tcp_pcb* clientTcp, TcpClientDataDelegate clientReceive, TcpClientCompleteDelegate onCompleted);
	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientEventDelegate onReadyToSend,
			  TcpClientDataDelegate onReceive = NULL);
	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientDataDelegate onReceive = NULL);
	TcpClient(TcpClientDataDelegate onReceive);
	virtual ~TcpClient();

public:
	virtual bool connect(String server, int port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual void close();

	/**	@brief	Set or clear the callback for received data
	 *	@param	receiveCb callback delegate or NULL
	 */
	void setReceiveDelegate(TcpClientDataDelegate receiveCb = NULL);

	/**	@brief	Set or clear the callback for connection close
	 *	@param	completeCb callback delegate or NULL
	 */
	void setCompleteDelegate(TcpClientCompleteDelegate completeCb = NULL);

	bool send(const char* data, uint16_t len, bool forceCloseAfterSent = false);
	bool sendString(const String& data, bool forceCloseAfterSent = false);
	__forceinline bool isProcessing()
	{
		return state == eTCS_Connected || state == eTCS_Connecting;
	}
	__forceinline TcpClientState getConnectionState()
	{
		return state;
	}

#ifdef ENABLE_SSL
	/**
	 * @brief Allows setting of multiple SSL validators after a successful handshake
	 * @param SslValidatorCallback callback
	 * @param void* data - The data that should be passed to the callback.
	 * 					   The callback will cast the data to the correct type and take care
	 * 					   to delete it.
	 *
	 */
	void addSslValidator(SslValidatorCallback callback, void* data = NULL);

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 * 			Check if SHA256 hash of Subject Public Key Info matches the one given.
	 * @note    For HTTP public key pinning (RFC7469), the SHA-256 hash of the
	 * 		    Subject Public Key Info (which usually only changes when the public key changes)
	 * 		    is used rather than the SHA-1 hash of the entire certificate
	 * 		    (which will change on each certificate renewal).
	 * @param const uint8_t *finterprint - the fingeprint data against which the match should be performed
	 * 									   The fingerprint will be deleted after use and should
	 * 									   not be reused outside of this method
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
	 *
	 * @return bool  true of success, false or failure
	 */
	bool pinCertificate(const uint8_t* fingerprint, SslFingerprintType type);

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 *
	 * @note  The data inside the fingerprints parameter is passed by reference
	 *
	 * @param SSLFingerprints - passes the certificate fingerprints by reference.
	 *
	 * @return bool  true of success, false or failure
	 */
	bool pinCertificate(SSLFingerprints fingerprints);
#endif

protected:
	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void onFinished(TcpClientState finishState);

#ifdef ENABLE_SSL
	virtual err_t onSslConnected(SSL* ssl);
#endif

	void pushAsyncPart();

protected:
	ReadWriteStream* stream = nullptr;

private:
	TcpClientState state;
	TcpClientCompleteDelegate completed = nullptr;
	TcpClientDataDelegate receive = nullptr;
	TcpClientEventDelegate ready = nullptr;

	bool asyncCloseAfterSent = false;
	int16_t asyncTotalSent = 0;
	int16_t asyncTotalLen = 0;
#ifdef ENABLE_SSL
	Vector<SslValidatorCallback> sslValidators;
	Vector<void*> sslValidatorsData;
#endif
};

/** @} */
#endif /* _SMING_CORE_TCPCLIENT_H_ */
