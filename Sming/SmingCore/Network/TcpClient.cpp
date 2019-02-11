/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
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

bool TcpClient::connect(const String& server, int port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if(isProcessing()) {
		return false;
	}

	state = eTCS_Connecting;
	return TcpConnection::connect(server.c_str(), port, useSsl, sslOptions);
}

bool TcpClient::connect(IPAddress addr, uint16_t port, boolean useSsl, uint32_t sslOptions)
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

	asyncTotalLen += len;
	asyncCloseAfterSent = forceCloseAfterSent;

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

	if(receive) {
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
		state = (asyncTotalSent == asyncTotalLen) ? eTCS_Successful : eTCS_Failed;
#ifdef ENABLE_SSL
		if(ssl && sslConnected) {
			state = (asyncTotalLen == 0 || (asyncTotalSent > asyncTotalLen)) ? eTCS_Successful : eTCS_Failed;
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
	if(stream == nullptr) {
		return;
	}

	write(stream);

	if(stream->isFinished()) {
		flush();
		debug_d("TcpClient stream finished");
		freeStreams();
	}
}

err_t TcpClient::onSent(uint16_t len)
{
	asyncTotalSent += len;

	if(stream == nullptr && asyncCloseAfterSent) {
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
	asyncTotalSent = 0;
	asyncTotalLen = 0;

	if(completed) {
		completed(*this, state == eTCS_Successful);
	}
}

#ifdef ENABLE_SSL
err_t TcpClient::onSslConnected(SSL* ssl)
{
	bool hasSuccess = (sslValidators.count() == 0);
	for(unsigned i = 0; i < sslValidators.count(); i++) {
		if(sslValidators[i](ssl, sslValidatorsData[i])) {
			hasSuccess = true;
			break;
		}
	}

	return hasSuccess ? ERR_OK : ERR_ABRT;
}

void TcpClient::addSslValidator(SslValidatorCallback callback, void* data)
{
	sslValidators.addElement(callback);
	sslValidatorsData.addElement(data);
}

bool TcpClient::pinCertificate(const uint8_t* fingerprint, SslFingerprintType type)
{
	SslValidatorCallback callback = nullptr;
	switch(type) {
	case eSFT_CertSha1:
		callback = sslValidateCertificateSha1;
		break;
	case eSFT_PkSha256:
		callback = sslValidatePublicKeySha256;
		break;
	default:
		debug_d("Unsupported SSL certificate fingerprint type");
	}

	if(!callback) {
		delete[] fingerprint;
		return false;
	}

	addSslValidator(callback, (void*)fingerprint);

	return true;
}

bool TcpClient::pinCertificate(const SslFingerprints& fingerprints)
{
	bool success = false;
	if(fingerprints.certSha1 != nullptr) {
		success = pinCertificate(fingerprints.certSha1, eSFT_CertSha1);
	}

	if(fingerprints.pkSha256 != nullptr) {
		success = pinCertificate(fingerprints.pkSha256, eSFT_PkSha256);
	}

	return success;
}
#endif
