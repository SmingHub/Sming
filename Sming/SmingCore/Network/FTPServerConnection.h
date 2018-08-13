/*
 * FTPServerConnection.h
 *
 *
 */

#ifndef SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_
#define SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_

#include "TcpConnection.h"
#include "IPAddress.h"
#include "WString.h"

/** @brief Maximum number of characters in an FTP command string
 */
#define MAX_FTP_CMD 255

class FTPServer;

enum FTPConnectionState { eFCS_Ready, eFCS_Authorization, eFCS_Active };

class FTPServerConnection : public TcpConnection {
	friend class FTPDataStream;
	friend class FTPServer;

public:
	FTPServerConnection(FTPServer* parentServer, tcp_pcb* clientTcp) :
		TcpConnection(clientTcp, true),
		_server(parentServer)
	{}

	virtual ~FTPServerConnection()
	{}

	virtual err_t onReceive(pbuf* buf);
	virtual err_t onSent(uint16_t len);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);

	void dataTransferFinished(TcpConnection* connection);

protected:
	virtual void onCommand(const String& cmd, const String& data);
	virtual void response(int code, const String& text = nullptr);
	int getSplitterPos(const String& data, char splitter, uint8_t number);
	String makeFileName(String name, bool shortIt);

	void cmdPort(const String& data);
	void createDataConnection(TcpConnection* connection);
	bool isCanTransfer()
	{
		return _canTransfer;
	}

private:
	FTPServer* _server = nullptr;
	FTPConnectionState _state = eFCS_Ready;
	String _userName = nullptr;
	String _renameFrom = nullptr;

	IPAddress _ip;
	uint16_t _port = 0;
	TcpConnection* _dataConnection = nullptr;
	bool _canTransfer = true;
};

#endif /* SMINGCORE_NETWORK_FTPSERVERCONNECTION_H_ */
