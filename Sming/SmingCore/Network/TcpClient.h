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

typedef Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)> TcpClientEventDelegate;
typedef Delegate<void(TcpClient& client, bool successful)> TcpClientCompleteDelegate;
typedef Delegate<bool(TcpClient& client, char* data, int size)> TcpClientDataDelegate;

enum TcpClientState { eTCS_Ready, eTCS_Connecting, eTCS_Connected, eTCS_Successful, eTCS_Failed };

// By default a TCP client connection has 70 seconds timeout
#define TCP_CLIENT_TIMEOUT 70

class TcpClient : public TcpConnection
{
public:
	TcpClient(bool autoDestruct) : TcpConnection(autoDestruct)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
	}

	TcpClient(tcp_pcb* clientTcp, TcpClientDataDelegate clientReceive, TcpClientCompleteDelegate onCompleted)
		: TcpConnection(clientTcp, true), state(eTCS_Connected)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
		completed = onCompleted;
		receive = clientReceive;
	}

	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientEventDelegate onReadyToSend,
			  TcpClientDataDelegate onReceive = nullptr)
		: TcpConnection(false)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
		completed = onCompleted;
		ready = onReadyToSend;
		receive = onReceive;
	}

	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientDataDelegate onReceive = nullptr) : TcpConnection(false)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
		completed = onCompleted;
		receive = onReceive;
	}

	TcpClient(TcpClientDataDelegate onReceive) : TcpConnection(false)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
		receive = onReceive;
	}

	virtual ~TcpClient()
	{
		freeStreams();
	}

public:
	virtual bool connect(const String& server, int port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual bool connect(IPAddress addr, uint16_t port, boolean useSsl = false, uint32_t sslOptions = 0);
	virtual void close();

	/**	@brief	Set or clear the callback for received data
	 *	@param	receiveCb callback delegate or nullptr
	 */
	void setReceiveDelegate(TcpClientDataDelegate receiveCb = nullptr)
	{
		receive = receiveCb;
	}

	/**	@brief	Set or clear the callback for connection close
	 *	@param	completeCb callback delegate or nullptr
	 */
	void setCompleteDelegate(TcpClientCompleteDelegate completeCb = nullptr)
	{
		completed = completeCb;
	}

	bool send(const char* data, uint16_t len, bool forceCloseAfterSent = false);

	bool sendString(const String& data, bool forceCloseAfterSent = false)
	{
		return send(data.c_str(), data.length(), forceCloseAfterSent);
	}

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
	void addSslValidator(SslValidatorCallback callback, void* data = nullptr)
	{
		sslValidators.add(callback, data);
	}

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
	bool pinCertificate(SSLFingerprints& fingerprints);
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
	void freeStreams();

protected:
	void setBuffer(ReadWriteStream* stream);

	ReadWriteStream* buffer = nullptr;   ///< Used internally to buffer arbitrary data via send() methods
	IDataSourceStream* stream = nullptr; ///< The currently active stream being sent

private:
	TcpClientState state = eTCS_Ready;
	TcpClientCompleteDelegate completed = nullptr;
	TcpClientDataDelegate receive = nullptr;
	TcpClientEventDelegate ready = nullptr;

	bool asyncCloseAfterSent = false;
	uint16_t asyncTotalSent = 0;
	uint16_t asyncTotalLen = 0;
#ifdef ENABLE_SSL
	SSLValidatorList sslValidators;
#endif
};

/** @} */
#endif /* _SMING_CORE_TCPCLIENT_H_ */
