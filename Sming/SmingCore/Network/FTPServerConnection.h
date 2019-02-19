/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServerConnection.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_FTP_SERVER_CONNECTION_H_
#define _SMING_CORE_NETWORK_FTP_SERVER_CONNECTION_H_

#include "TcpConnection.h"
#include "IPAddress.h"
#include "WString.h"

#define MAX_FTP_CMD 255

class FTPServer;

enum FTPConnectionState { eFCS_Ready, eFCS_Authorization, eFCS_Active };

class FTPServerConnection : public TcpConnection
{
	friend class FTPDataStream;
	friend class FTPServer;

public:
	FTPServerConnection(FTPServer* parentServer, tcp_pcb* clientTcp)
		: TcpConnection(clientTcp, true), server(parentServer), state(eFCS_Ready)
	{
	}

	err_t onReceive(pbuf* buf) override;
	err_t onSent(uint16_t len) override;
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	void dataTransferFinished(TcpConnection* connection);

protected:
	virtual void onCommand(String cmd, String data);
	virtual void response(int code, String text = "");

	int getSplitterPos(String data, char splitter, uint8_t number);
	String makeFileName(String name, bool shortIt);

	void cmdPort(const String& data);
	void createDataConnection(TcpConnection* connection);

	bool isCanTransfer()
	{
		return canTransfer;
	}

private:
	FTPServer* server;
	FTPConnectionState state;
	String userName;
	String renameFrom;

	IPAddress ip;
	int port = 0;
	TcpConnection* dataConnection = nullptr;
	bool canTransfer = true;
};

#endif /* _SMING_CORE_NETWORK_FTP_SERVER_CONNECTION_H_ */
