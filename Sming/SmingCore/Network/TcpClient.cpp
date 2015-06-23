/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpClient.h"
#include "../Wiring/IPAddress.h"
#include "../../SmingCore/DataSourceStream.h"
#include "../../Wiring/WString.h"

TcpClient::TcpClient(tcp_pcb *clientTcp, TcpClientDataDelegate clientReceive, bool autoDestruct )
: TcpConnection(clientTcp, autoDestruct), state(eTCS_Connected), asyncTotalSent(0), asyncTotalLen(0)
{
	completed = NULL;
	ready = NULL;
	receive = clientReceive;
	stream = NULL;
}

TcpClient::TcpClient(bool autoDestruct)
	: TcpConnection(autoDestruct), state(eTCS_Ready), asyncTotalSent(0), asyncTotalLen(0), asyncCloseAfterSent(false)
{
	completed = NULL;
	ready = NULL;
	receive = NULL;
	stream = NULL;
}

TcpClient::TcpClient(TcpClientBoolDelegate onCompleted, TcpClientEventDelegate onReadyToSend /* = NULL*/, TcpClientDataDelegate onReceive /* = NULL*/)
	: TcpConnection(false), state(eTCS_Ready), asyncTotalSent(0), asyncTotalLen(0), asyncCloseAfterSent(false)
{
	completed = onCompleted;
	ready = onReadyToSend;
	receive = onReceive;
	stream = NULL;
}

TcpClient::TcpClient(TcpClientBoolDelegate onCompleted, TcpClientDataDelegate onReceive /* = NULL*/)
	: TcpConnection(false), state(eTCS_Ready), ready(NULL), asyncTotalSent(0), asyncTotalLen(0), asyncCloseAfterSent(false)
{
	completed = onCompleted;
	receive = onReceive;
	stream = NULL;
}


TcpClient::TcpClient(TcpClientDataDelegate onReceive)
	: TcpConnection(false), state(eTCS_Ready), ready(NULL), completed(NULL), asyncTotalSent(0), asyncTotalLen(0), asyncCloseAfterSent(false)
{
	receive = onReceive;
	stream = NULL;
}

TcpClient::~TcpClient()
{
	if (stream != NULL)
	{
		delete[] stream;
		stream = NULL;
	}
}

bool TcpClient::connect(String server, int port)
{
	if (isProcessing()) return false;

	state = eTCS_Connecting;
	return TcpConnection::connect(server.c_str(), port);
}

bool TcpClient::connect(IPAddress addr, uint16_t port)
{
	if (isProcessing()) return false;

	state = eTCS_Connecting;
	return TcpConnection::connect(addr, port);
}

bool TcpClient::sendString(String data, bool forceCloseAfterSent /* = false*/)
{
	return send(data.c_str(), data.length(), forceCloseAfterSent);
}

bool TcpClient::send(const char* data, uint8_t len, bool forceCloseAfterSent /* = false*/)
{
	if (state != eTCS_Connecting && state != eTCS_Connected) return false;

	if (stream == NULL)
		stream = new MemoryDataStream();

	stream->write((const uint8_t*)data, len);
	asyncTotalLen += len;
	asyncCloseAfterSent = forceCloseAfterSent;

	return true;
}

err_t TcpClient::onConnected(err_t err)
{
	if (err == ERR_OK)
	{
		state = eTCS_Connected;
	}
	else
	{
		onError(err);
	}

	// Fire ReadyToSend callback
	TcpConnection::onConnected(err);

	return ERR_OK;
}

err_t TcpClient::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		TcpConnection::onReceive(buf);
	}
	else
	{
		if (receive)
		{
			char* data = new char[buf->tot_len + 1];
			pbuf_copy_partial(buf, data, buf->tot_len, 0);
			data[buf->tot_len] = '\0';

			if (!receive(*this, data, buf->tot_len))
			{
				delete[] data;
				return ERR_MEM;
			}

			delete[] data;
		}

		// Fire ReadyToSend callback
		TcpConnection::onReceive(buf);
	}

	return ERR_OK;
}

void TcpClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	TcpConnection::onReadyToSendData(sourceEvent);
	if (ready)
		ready(*this, sourceEvent);

	pushAsyncPart();
}

void TcpClient::close()
{
	if (state != eTCS_Successful && state != eTCS_Failed)
	{
		state = (asyncTotalSent == asyncTotalLen) ? eTCS_Successful : eTCS_Failed;
		asyncTotalLen = 0;
		asyncTotalSent = 0;
		onFinished(state);
	}

	// Close connection only after processing
	TcpConnection::close();
}

void TcpClient::pushAsyncPart()
{
	if (stream == NULL) return;

	write(stream);

	if (stream->isFinished())
	{
		flush();
		debugf("TcpClient request completed");
		delete stream; // Free memory now!
		stream = NULL;
	}
}

err_t TcpClient::onSent(uint16_t len)
{
	asyncTotalSent += len;

	if (stream == NULL && asyncCloseAfterSent)
	{
		TcpConnection::onSent(len);
		close();
	}
	else
	{
		// Fire ReadyToSend callback
		TcpConnection::onSent(len);
	}

	return ERR_OK;
}

void TcpClient::onError(err_t err)
{
	state = eTCS_Failed;
	onFinished(state);

	TcpConnection::onError(err);
}

void TcpClient::onFinished(TcpClientState finishState)
{
	if (stream != NULL)
		delete stream; // Free memory now!
	stream = NULL;

	if (completed)
		completed(*this, state == eTCS_Successful);
}
