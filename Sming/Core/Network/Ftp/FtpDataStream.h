/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * FtpDataStream.h
 *
 ****/

#pragma once

#include "FtpServerConnection.h"
#include "Network/TcpConnection.h"

/*
	RFC959:

	User-DTP listens on data port when required.

	Server is responsible for creating the data connection.

	The server MUST close the data connection under the following conditions:

		1. The server has completed sending data in a transfer mode
		that requires a close to indicate EOF.

		2. The server receives an ABORT command from the user.

		3. The port specification is changed by a command from the user.

		4. The control connection is closed legally or otherwise.

		5. An irrecoverable error condition occurs.

	Otherwise the close is a server option, the exercise of which the
	server must indicate to the user-process by either a 250 or 226
	reply only.
*/
class FtpDataStream : public TcpConnection
{
public:
	explicit FtpDataStream(FtpServerConnection& control) : TcpConnection(true), control(control)
	{
	}

	~FtpDataStream()
	{
		control.dataStreamDestroyed(this);
	}

	err_t onConnected(err_t err) override
	{
		setTimeOut(300);
		return TcpConnection::onConnected(err);
	}

	void finishTransfer()
	{
		close();
		control.dataTransferFinished(this);
	}

	void response(int code, String text = nullptr)
	{
		control.response(code, text);
	}

	void onReadyToSendData(TcpConnectionEvent sourceEvent) override
	{
		if(completed) {
			finishTransfer();
		} else {
			transferData(sourceEvent);
		}
	}

	virtual void transferData(TcpConnectionEvent sourceEvent)
	{
	}

protected:
	FtpServerConnection& control;
	bool completed{false};
};
