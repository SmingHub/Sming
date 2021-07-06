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
#include "Data/Stream/StreamChain.h"

void TcpClient::freeStreams()
{
	delete stream;
	stream = nullptr;
}

bool TcpClient::connect(const String& server, int port, bool useSsl)
{
	if(isProcessing()) {
		return false;
	}

	state = eTCS_Connecting;
	return TcpConnection::connect(server.c_str(), port, useSsl);
}

bool TcpClient::connect(IpAddress addr, uint16_t port, bool useSsl)
{
	if(isProcessing()) {
		return false;
	}

	state = eTCS_Connecting;
	return TcpConnection::connect(addr, port, useSsl);
}

bool TcpClient::send(const char* data, uint16_t len, bool forceCloseAfterSent)
{
	if(state != eTCS_Connecting && state != eTCS_Connected) {
		return false;
	}

	auto memoryStream = static_cast<MemoryDataStream*>(stream);
	if(memoryStream == nullptr || memoryStream->getStreamType() != eSST_MemoryWritable) {
		memoryStream = new MemoryDataStream();
	}

	if(memoryStream->write(data, len) != len) {
		debug_e("TcpClient::send ERROR: Unable to store %d bytes in buffer", len);
		return false;
	}

	return send(memoryStream, forceCloseAfterSent);
}

bool TcpClient::send(IDataSourceStream* source, bool forceCloseAfterSent)
{
	if(state != eTCS_Connecting && state != eTCS_Connected) {
		return false;
	}

	if(source == nullptr) {
		return false;
	}

	if(stream == nullptr) {
		stream = source;
	}
	else if(stream != source){
		auto chainStream = static_cast<StreamChain*>(stream);
		if(chainStream != nullptr && chainStream->getStreamType() == eSST_Chain) {
			if(!chainStream->attachStream(source)) {
				debug_w("Unable to attach source to existing stream chain!");
				delete source;
				return false;
			}
		}
		else {
			debug_d("Creating stream chain ...");
			chainStream  = new StreamChain();
			if(!chainStream) {
				delete source;
				debug_w("Unable to create stream chain!");
				return false;
			}

			if(!chainStream->attachStream(stream)) {
				delete source;
				delete chainStream;
				debug_w("Unable to attach stream to new chain!");
				return false;
			}

			if(!chainStream->attachStream(source)) {
				delete source;
				delete chainStream;
				debug_w("Unable to attach source to new chain!");
				return false;
			}

			stream = chainStream;
		}
	}

	int length = source->available();
	if(length > 0) {
		totalSentBytes += length;
	}

	debug_d("Sending stream. Bytes to send: %d", length);

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
		state = (totalSentConfirmedBytes >= totalSentBytes) ? eTCS_Successful : eTCS_Failed;
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
		trySend(eTCE_Poll);
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

void TcpClient::onClosed()
{
	onFinished(state);
	TcpConnection::onClosed();
	state = eTCS_Ready;
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
