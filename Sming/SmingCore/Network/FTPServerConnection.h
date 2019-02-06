#ifndef SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_
#define SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_

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

	virtual ~FTPServerConnection()
	{
	}

	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

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

#endif /* SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_ */
