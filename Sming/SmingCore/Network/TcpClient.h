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

//typedef void (*TcpClientEventDelegate)(TcpClient& client, TcpConnectionEvent sourceEvent);
//typedef void (*TcpClientBoolDelegate)(TcpClient& client, bool successful);
//typedef bool (*TcpClientDataDelegate)(TcpClient& client, char *data, int size);

typedef Delegate<void(TcpClient& client, TcpConnectionEvent sourceEvent)> TcpClientEventDelegate;
typedef Delegate<void(TcpClient& client, bool successful)> TcpClientCompleteDelegate;
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
	TcpClient(tcp_pcb *clientTcp, TcpClientDataDelegate clientReceive, TcpClientCompleteDelegate onCompleted);
	TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientEventDelegate onReadyToSend, TcpClientDataDelegate onReceive = NULL);
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
	TcpClientCompleteDelegate completed = nullptr;
	TcpClientDataDelegate receive = nullptr;
	TcpClientEventDelegate ready = nullptr;
	MemoryDataStream* stream = nullptr;
	bool asyncCloseAfterSent = false;
	int16_t asyncTotalSent = 0;
	int16_t asyncTotalLen = 0;
};

#endif /* _SMING_CORE_TCPCLIENT_H_ */
