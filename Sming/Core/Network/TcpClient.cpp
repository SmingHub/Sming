/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TcpClient.cpp
 *
 ****/

#include "TcpClient.h"
#include "Data/Stream/MemoryDataStream.h"

void TcpClient::freeStreams()
{
	if(buffer != nullptr) {
		if(buffer != stream) {
			debug_e("TcpClient: buffer doesn't match stream");
			delete buffer;
		}
		buffer = nullptr;
	}

	delete stream;
	stream = nullptr;
}

void TcpClient::setBuffer(ReadWriteStream* stream)
{
	freeStreams();
	buffer = stream;
	this->stream = buffer;
}

bool TcpClient::connect(const String& server, int port, bool useSsl, uint32_t sslOptions)
{
	if(isProcessing()) {
		return false;
	}

	state = eTCS_Connecting;
	return TcpConnection::connect(server.c_str(), port, useSsl, sslOptions);
}

bool TcpClient::connect(IpAddress addr, uint16_t port, bool useSsl, uint32_t sslOptions)
{
	if(isProcessing()) {
		return false;
	}

	state = eTCS_Connecting;
	return TcpConnection::connect(addr, port, useSsl, sslOptions);
}

bool TcpClient::send(const char* data, uint16_t len, bool forceCloseAfterSent)
{
	if(state != eTCS_Connecting && state != eTCS_Connected) {
		return false;
	}

	if(buffer == nullptr) {
		setBuffer(new MemoryDataStream());
		if(buffer == nullptr) {
			return false;
		}
	}

	if(buffer->write((const uint8_t*)data, len) != len) {
		debug_e("TcpClient::send ERROR: Unable to store %d bytes in buffer", len);
		return false;
	}

	debug_d("Storing %d bytes in stream", len);

	totalSentBytes += len;
	closeAfterSent = forceCloseAfterSent ? eTCCASS_AfterSent : eTCCASS_None;

	return true;
}

err_t TcpClient::onConnected(err_t err)
{
	if(err == ERR_OK) {
		state = eTCS_Connected;
	} else {
		onError(err);
	}

	// Fire ReadyToSend callback
	TcpConnection::onConnected(err);

	return ERR_OK;
}

err_t TcpClient::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		// Disconnected, close it
		return TcpConnection::onReceive(buf);
	}

	bool success = true;
	if(closeAfterSent == eTCCASS_AfterSent_Ignore_Received) {
		success = false;
	}

	if(success && receive) {
		pbuf* cur = buf;
		while(cur != nullptr && cur->len > 0) {
			bool success = receive(*this, (char*)cur->payload, cur->len);
			if(!success) {
				debug_d("TcpClient::onReceive: Aborted from receive callback");

				TcpConnection::onReceive(nullptr);
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
	if(ready) {
		ready(*this, sourceEvent);
	}

	pushAsyncPart();
}

void TcpClient::close()
{
	if(state != eTCS_Successful && state != eTCS_Failed) {
		state = (totalSentConfirmedBytes == totalSentBytes) ? eTCS_Successful : eTCS_Failed;
		if(ssl && sslConnected) {
			state = (totalSentBytes == 0 || (totalSentConfirmedBytes > totalSentBytes)) ? eTCS_Successful : eTCS_Failed;
		}
		totalSentBytes = 0;
		totalSentConfirmedBytes = 0;
		onFinished(state);
	}

	// Close connection only after processing
	TcpConnection::close();
}

void TcpClient::pushAsyncPart()
{
	if(stream == nullptr) {
		return;
	}

	write(stream);

	if(stream->isFinished()) {
		debug_d("TcpClient stream finished");
		freeStreams();

		if(getAvailableWriteSize() > 0) {
			// if there is space in the output buffer
			// then don't wait for tcp sent confirmation and try sending more data now
			onReadyToSendData(TcpConnectionEvent::eTCE_Poll);
		}

		flush();
	}
}

err_t TcpClient::onSent(uint16_t len)
{
	totalSentConfirmedBytes += len;

	if(stream == nullptr && closeAfterSent != eTCCASS_None) {
		TcpConnection::onSent(len);
		close();
	} else {
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
	freeStreams();

	// Initialize async variables for next connection
	totalSentConfirmedBytes = 0;
	totalSentBytes = 0;

	if(completed) {
		completed(*this, state == eTCS_Successful);
	}
}
