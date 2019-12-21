/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpClient.h
 *
 ****/

/** @defgroup   tcpclient Clients
 *  @brief      Provides base TCP client
 *  @ingroup    tcp
 *  @{
 */

#pragma once

#include "TcpConnection.h"
#include <Network/Ssl/Validator.h>

class TcpClient;
class ReadWriteStream;
class IpAddress;

typedef Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)> TcpClientEventDelegate;
typedef Delegate<void(TcpClient& client, bool successful)> TcpClientCompleteDelegate;
typedef Delegate<bool(TcpClient& client, char* data, int size)> TcpClientDataDelegate;

enum TcpClientState { eTCS_Ready, eTCS_Connecting, eTCS_Connected, eTCS_Successful, eTCS_Failed };
enum TcpClientCloseAfterSentState { eTCCASS_None, eTCCASS_AfterSent, eTCCASS_AfterSent_Ignore_Received };

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
		: TcpConnection(clientTcp, true), state(eTCS_Connected), completed(onCompleted), receive(clientReceive)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
	}

	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientEventDelegate onReadyToSend,
			  TcpClientDataDelegate onReceive = nullptr)
		: TcpConnection(false), completed(onCompleted), ready(onReadyToSend), receive(onReceive)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
	}

	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientDataDelegate onReceive = nullptr)
		: TcpConnection(false), completed(onCompleted), receive(onReceive)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
	}

	explicit TcpClient(TcpClientDataDelegate onReceive) : TcpConnection(false), receive(onReceive)
	{
		TcpConnection::timeOut = TCP_CLIENT_TIMEOUT;
	}

	~TcpClient()
	{
		freeStreams();
	}

public:
	bool connect(const String& server, int port, bool useSsl = false, uint32_t sslOptions = 0) override;
	bool connect(IpAddress addr, uint16_t port, bool useSsl = false, uint32_t sslOptions = 0) override;
	void close() override;

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

	bool isProcessing()
	{
		return state == eTCS_Connected || state == eTCS_Connecting;
	}

	TcpClientState getConnectionState()
	{
		return state;
	}

	/**
	 * Schedules the connection to get closed after the data is sent
	 * @param ignoreIncomingData when that flag is set the connection will start ignoring incoming data.
	 */
	void setCloseAfterSent(bool ignoreIncomingData = false)
	{
		closeAfterSent = ignoreIncomingData ? eTCCASS_AfterSent_Ignore_Received : eTCCASS_AfterSent;
	}

	/**
	 * @brief Allows setting of multiple SSL validators after a successful handshake
	 * @param callback The callback function to be invoked on validation
	 * @param data The data to pass to the callback
	 * @note The callback is responsible for releasing the data if appropriate.
	 * See Ssl::Validator::Callback for further details.
	 *
	 * @retval bool true on success, false on failure
	 */
	bool addSslValidator(Ssl::Validator::Callback callback, void* data = nullptr)
	{
		return sslValidators.add(callback, data);
	}

	/**
	 * @brief Requires (pins) the remote SSL certificate to match certain fingerprints
	 * @param fingerprint	The fingerprint data against which the match should be performed.
	 * 						Must be allocated on the heap and will be deleted after use.
	 * 						Do not re-use outside of this method.
	 * @param type			The fingerprint type - see Ssl::FingerprintType for details.
	 *
	 * @retval bool true on success, false on failure
	 */
	bool pinCertificate(const uint8_t* fingerprint, Ssl::FingerprintType type)
	{
		return sslValidators.add(fingerprint, type);
	}

	/**
	 * @brief	Requires (pins) the remote SSL certificate to match certain fingerprints
	 * @note	The data inside the fingerprints parameter is passed by reference
	 * @param	fingerprints - passes the certificate fingerprints by reference.
	 *
	 * @retval bool  true on success, false on failure
	 */
	bool pinCertificate(Ssl::Fingerprints& fingerprints)
	{
		return sslValidators.add(fingerprints);
	}

protected:
	err_t onConnected(err_t err) override;
	err_t onReceive(pbuf* buf) override;
	err_t onSent(uint16_t len) override;
	void onError(err_t err) override;
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	virtual void onFinished(TcpClientState finishState);

	err_t onSslConnected(Ssl::Connection* connection) override
	{
		if(connection == nullptr) {
			return ERR_ABRT;
		}

		return sslValidators.validate(&connection->getCertificate()) ? ERR_OK : ERR_ABRT;
	}

	void pushAsyncPart();
	void freeStreams();

protected:
	void setBuffer(ReadWriteStream* stream);

	ReadWriteStream* buffer = nullptr;   ///< Used internally to buffer arbitrary data via send() methods
	IDataSourceStream* stream = nullptr; ///< The currently active stream being sent

private:
	TcpClientState state = eTCS_Ready;
	TcpClientCompleteDelegate completed = nullptr;
	TcpClientEventDelegate ready = nullptr;
	TcpClientDataDelegate receive = nullptr;

	TcpClientCloseAfterSentState closeAfterSent = eTCCASS_None;
	uint16_t totalSentConfirmedBytes = 0;
	uint16_t totalSentBytes = 0;

	Ssl::ValidatorList sslValidators;
};

/** @} */
