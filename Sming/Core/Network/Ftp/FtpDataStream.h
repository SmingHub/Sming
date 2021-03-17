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

class FtpDataStream : public TcpConnection
{
public:
	explicit FtpDataStream(FtpServerConnection& control) : TcpConnection(true), control(control)
	{
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
