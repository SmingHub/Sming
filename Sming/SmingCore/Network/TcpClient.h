/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_TCPCLIENT_H_
#define _SMING_CORE_TCPCLIENT_H_

#include "TcpConnection.h"
#include "../Delegate.h"

class TcpClient;
class MemoryDataStream;
class IPAddress;

//typedef void (*TcpClientEventCallback)(TcpClient& client, TcpConnectionEvent sourceEvent);
//typedef void (*TcpClientBoolCallback)(TcpClient& client, bool successful);
//typedef bool (*TcpClientDataCallback)(TcpClient& client, char *data, int size);

typedef Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)> TcpClientEventDelegate;
typedef Delegate<void(TcpClient& client, bool successful)> TcpClientBoolDelegate;
typedef Delegate<bool(TcpClient& client, char *data, int size)> TcpClientDataDelegate;

enum TcpClientState
{
	eTCS_Ready,
	eTCS_Connecting,
	eTCS_Connected,
	eTCS_Successful,
	eTCS_Failed
};

class TcpClient : public TcpConnection
{
public:
	TcpClient(bool autoDestruct);
	TcpClient(tcp_pcb *clientTcp, TcpClientDataDelegate clientReceive, bool autoDestruct);
	TcpClient(TcpClientBoolDelegate onCompleted, TcpClientEventDelegate onReadyToSend, TcpClientDataDelegate onReceive = NULL);
	TcpClient(TcpClientBoolDelegate onCompleted, TcpClientDataDelegate onReceive = NULL);
	TcpClient(TcpClientDataDelegate onReceive);
	virtual ~TcpClient();

public:
	virtual bool connect(String server, int port);
	virtual bool connect(IPAddress addr, uint16_t port);
	virtual void close();

	bool send(const char* data, uint8_t len, bool forceCloseAfterSent = false);
	bool sendString(String data, bool forceCloseAfterSent = false);
	__forceinline bool isProcessing()  { return state == eTCS_Connected || state == eTCS_Connecting; }
	__forceinline TcpClientState getConnectionState() { return state; }

protected:
	virtual err_t onConnected(err_t err);
	virtual err_t onReceive(pbuf *buf);
	virtual err_t onSent(uint16_t len);
	virtual void onError(err_t err);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void onFinished(TcpClientState finishState);

	void pushAsyncPart();

private:
	TcpClientState state;
	TcpClientBoolDelegate completed;
	TcpClientDataDelegate receive;
	TcpClientEventDelegate ready;
	MemoryDataStream* stream;
	bool asyncCloseAfterSent;
	int16_t asyncTotalSent;
	int16_t asyncTotalLen;
};

#endif /* _SMING_CORE_TCPCLIENT_H_ */
