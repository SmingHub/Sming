/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataStream.h
 *
 ****/

#ifndef _SMING_CORE_NETWORK_FTP_FTP_DATA_STREAM_H_
#define _SMING_CORE_NETWORK_FTP_FTP_DATA_STREAM_H_

#include "FtpServerConnection.h"
#include "TcpConnection.h"

class FtpDataStream : public TcpConnection
{
public:
	explicit FtpDataStream(FtpServerConnection* connection) : TcpConnection(true), parent(connection)
	{
	}

	err_t onConnected(err_t err) override
	{
		//response(125, "Connected");
		setTimeOut(300); // Update timeout
		return TcpConnection::onConnected(err);
	}

	err_t onSent(uint16_t len) override
	{
		sent += len;
		if(written < sent || !completed) {
			return TcpConnection::onSent(len);
		}
		finishTransfer();
		return TcpConnection::onSent(len);
	}

	void finishTransfer()
	{
		close();
		parent->dataTransferFinished(this);
	}

	void response(int code, String text = nullptr)
	{
		parent->response(code, text);
	}

	int write(const char* data, int len, uint8_t apiflags = 0)
	{
		written += len;
		return TcpConnection::write(data, len, apiflags);
	}

	void onReadyToSendData(TcpConnectionEvent sourceEvent) override
	{
		if(!parent->isCanTransfer()) {
			return;
		}
		if(completed && written == 0) {
			finishTransfer();
		}
		transferData(sourceEvent);
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
	}

protected:
	FtpServerConnection* parent = nullptr;
	bool completed = false;
	unsigned written = 0;
	unsigned sent = 0;
};

#endif /* _SMING_CORE_NETWORK_FTP_FTP_DATA_STREAM_H_ */
