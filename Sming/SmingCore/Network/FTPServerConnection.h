#ifndef SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_
#define SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_

#include "TcpConnection.h"
#include "../../Wiring/IPAddress.h"
#include "../Wiring/WString.h"

#define MAX_FTP_CMD 255

class FTPServer;

enum FTPConnectionState
{
	eFCS_Ready,
	eFCS_Authorization,
	eFCS_Active
};

class FTPServerConnection : public TcpConnection
{
	friend class FTPDataStream;
	friend class FTPServer;
public:
	FTPServerConnection(FTPServer *parentServer, tcp_pcb *clientTcp);
	virtual ~FTPServerConnection();

	virtual err_t onReceive(pbuf *buf);
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
	bool isCanTransfer() { return canTransfer; }

private:
	FTPServer *server;
	FTPConnectionState state;
	String userName;
	String renameFrom;

	IPAddress ip;
	int port;
	TcpConnection *dataConnection;
	bool canTransfer;
};

#endif /* SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_ */
