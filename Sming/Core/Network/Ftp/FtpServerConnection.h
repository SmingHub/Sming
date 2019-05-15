/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpServerConnection.h
 *
 ****/

#pragma once

#include "Network/TcpConnection.h"
#include "IPAddress.h"
#include "WString.h"

#define MAX_FTP_CMD 255

class FtpServer;

enum FtpConnectionState { eFCS_Ready, eFCS_Authorization, eFCS_Active };

class FtpServerConnection : public TcpConnection
{
	friend class FtpDataStream;
	friend class FtpServer;

public:
	FtpServerConnection(FtpServer* parentServer, tcp_pcb* clientTcp)
		: TcpConnection(clientTcp, true), server(parentServer)
	{
	}

	err_t onReceive(pbuf* buf) override;
	err_t onSent(uint16_t len) override;
	void onReadyToSendData(TcpConnectionEvent sourceEvent) override;

	void dataTransferFinished(TcpConnection* connection);

protected:
	virtual void onCommand(String cmd, String data);
	virtual void response(int code, String text = "");

	int getSplitterPos(const String& data, char splitter, uint8_t number);
	String makeFileName(String name, bool shortIt);

	void cmdPort(const String& data);
	void createDataConnection(TcpConnection* connection);

	bool isCanTransfer()
	{
		return canTransfer;
	}

private:
	FtpServer* server = nullptr;
	FtpConnectionState state = eFCS_Ready;
	String userName;
	String renameFrom;

	IPAddress ip;
	int port = 0;
	TcpConnection* dataConnection = nullptr;
	bool canTransfer = true;
};

typedef FtpServerConnection FTPServerConnection SMING_DEPRECATED; // @deprecated Use `FtpServerConnection` instead
