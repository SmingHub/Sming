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

class TcpClient;
class ReadWriteStream;
class IpAddress;

using TcpClientEventDelegate = Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)>;
using TcpClientCompleteDelegate = Delegate<void(TcpClient& client, bool successful)>;
using TcpClientDataDelegate = Delegate<bool(TcpClient& client, char* data, int size)>;

enum TcpClientState {
	eTCS_Ready,
	eTCS_Connecting,
	eTCS_Connected,
	eTCS_Successful,
	eTCS_Failed,
};

enum TcpClientCloseAfterSentState {
	eTCCASS_None,
	eTCCASS_AfterSent,
	eTCCASS_AfterSent_Ignore_Received,
};

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

	bool connect(const String& server, int port, bool useSsl = false) override;
	bool connect(IpAddress addr, uint16_t port, bool useSsl = false) override;
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

protected:
	err_t onConnected(err_t err) override;
	err_t onReceive(pbuf* buf) override;
	err_t onSent(uint16_t len) override;
	void onError(err_t err) override;
	void onClosed() override;

	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	virtual void onFinished(TcpClientState finishState);

	void pushAsyncPart();
	void freeStreams();

protected:
	void setBuffer(ReadWriteStream* stream);

	ReadWriteStream* buffer = nullptr;   ///< Used internally to buffer arbitrary data via send() methods
	IDataSourceStream* stream = nullptr; ///< The currently active stream being sent

private:
	TcpClientState state = eTCS_Ready;
	TcpClientCompleteDelegate completed;
	TcpClientEventDelegate ready;
	TcpClientDataDelegate receive;

	TcpClientCloseAfterSentState closeAfterSent = eTCCASS_None;
	uint16_t totalSentConfirmedBytes = 0;
	uint16_t totalSentBytes = 0;
};

/** @} */
