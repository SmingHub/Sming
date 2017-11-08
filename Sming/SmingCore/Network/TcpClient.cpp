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

TcpClient::TcpClient(tcp_pcb *clientTcp, TcpClientDataDelegate clientReceive, TcpClientCompleteDelegate onCompleted)
: TcpConnection(clientTcp, true), state(eTCS_Connected)
{
	completed = onCompleted;
	receive = clientReceive;
}

TcpClient::TcpClient(bool autoDestruct)
	: TcpConnection(autoDestruct), state(eTCS_Ready)
{
}

TcpClient::TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientEventDelegate onReadyToSend /* = NULL*/, TcpClientDataDelegate onReceive /* = NULL*/)
	: TcpConnection(false), state(eTCS_Ready)
{
	completed = onCompleted;
	ready = onReadyToSend;
	receive = onReceive;
}

TcpClient::TcpClient(TcpClientCompleteDelegate onCompleted, TcpClientDataDelegate onReceive /* = NULL*/)
	: TcpConnection(false), state(eTCS_Ready)
{
	completed = onCompleted;
	receive = onReceive;
}


TcpClient::TcpClient(TcpClientDataDelegate onReceive)
	: TcpConnection(false), state(eTCS_Ready)
{
	receive = onReceive;
}

TcpClient::~TcpClient()
{
	if (stream != NULL)
	{
		delete[] stream;
		stream = NULL;
	}
}

bool TcpClient::connect(String server, int port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (isProcessing()) return false;

	state = eTCS_Connecting;
	return TcpConnection::connect(server.c_str(), port, useSsl, sslOptions);
}

bool TcpClient::connect(IPAddress addr, uint16_t port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (isProcessing()) return false;

	state = eTCS_Connecting;
	return TcpConnection::connect(addr, port, useSsl, sslOptions);
}

bool TcpClient::sendString(const String& data, bool forceCloseAfterSent /* = false*/)
{
	return send(data.c_str(), data.length(), forceCloseAfterSent);
}

bool TcpClient::send(const char* data, uint16_t len, bool forceCloseAfterSent /* = false*/)
{
	if (state != eTCS_Connecting && state != eTCS_Connected) return false;

	if (stream == NULL)
		stream = new MemoryDataStream();

	if (stream->write((const uint8_t*)data, len) != len) {
		debug_e("ERROR: Unable to store %d bytes in output stream", len);
		return false;
	}

	debugf("Storing %d bytes in stream", len);

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
		return TcpConnection::onReceive(buf);
	}

	if (receive)
	{
		pbuf *cur = buf;
		while (cur != NULL && cur->len > 0) {
			bool success = !receive(*this, (char*)cur->payload, cur->len);
			if(!success) {
				debugf("TcpClient::onReceive: Aborted from receive callback");

				TcpConnection::onReceive(NULL);
				return ERR_ABRT; // abort the connection
			}

			cur = cur->next;
		}
	}

	// Fire ReadyToSend callback
	TcpConnection::onReceive(buf);

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
#ifdef ENABLE_SSL
		if(ssl && sslConnected) {
			state = (asyncTotalLen==0 || (asyncTotalSent > asyncTotalLen)) ? eTCS_Successful : eTCS_Failed;
		}
#endif
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
		debugf("TcpClient stream finished");
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
	// Initialize async variables for next connection
	asyncTotalSent = 0;
	asyncTotalLen = 0;

	if (completed)
		completed(*this, state == eTCS_Successful);
}

void TcpClient::setReceiveDelegate(TcpClientDataDelegate receiveCb)
{
	receive = receiveCb;
}

void TcpClient::setCompleteDelegate(TcpClientCompleteDelegate completeCb)
{
	completed = completeCb;
}
