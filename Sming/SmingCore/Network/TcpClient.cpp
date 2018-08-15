/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "TcpClient.h"
#include "Data/Stream/MemoryDataStream.h"

TcpClient::~TcpClient()
{
	freeStreams();
}

void TcpClient::freeStreams()
{
	if (_buffer) {
		if (_buffer != _stream)
			debug_e("TcpClient: _buffer doesn't match _stream");
		delete _buffer;
		_buffer = nullptr;
	}

	delete _stream;
	_stream = nullptr;
}

bool TcpClient::connect(const String& server, int port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (isProcessing())
		return false;

	_state = eTCS_Connecting;
	return TcpConnection::connect(server, port, useSsl, sslOptions);
}

bool TcpClient::connect(IPAddress addr, uint16_t port, boolean useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	if (isProcessing())
		return false;

	_state = eTCS_Connecting;
	return TcpConnection::connect(addr, port, useSsl, sslOptions);
}

/*
 * 13/8/2018 (mikee47)
 *
 * Several classes access _stream directly, hence it's protected:
 *
 * 	HttpConnection
 * 	HttpServerConnection
 * 	SmtpClient
 *
 * In send(const char*, ...) below we also construct our own memory stream to allow
 * users to write arbitrary data.
 *
 * Users use one or other of these approaches; we never provide a stream, and then
 * use send(const char*) on it. There are some checks on this but it's clumsy and
 * requires casting. There is a similar situation in HttpResponse, which is now
 * handled by creating a separate _buffer stream for writing, and referenced by
 * the IDataSourceStream member for reading back out again. There are also proper
 * checks in place to catch incorrect usage.
 *
 * We need to do the same here. So:
 *
 * 	Make _stream private
 * 	Use this new method to submit a stream for transmission
 *  Change send(char*) method to use _buffer and add additional checks.
 *  _stream is freed when transmission is completed so we can add a method which
 *  simply checks for
 *  Modify above three clients to use these methods.
 *
 */
bool TcpClient::send(IDataSourceStream* stream, bool forceCloseAfterSent)
{
	if (!stream) {
		debug_e("TcpClient::send() ERROR - supplied stream is null");
		return false;
	}

	if (_stream) {
		debug_w("TcpClient::send() - existing stream being discarded");
		freeStreams();
	}

	_stream = stream;

	_asyncTotalLen += stream->available();
	_asyncCloseAfterSent = forceCloseAfterSent;

	return true;
}

bool TcpClient::send(const char* data, uint16_t len, bool forceCloseAfterSent /* = false*/)
{
	if (_state != eTCS_Connecting && _state != eTCS_Connected)
		return false;

	if (_buffer) {
		// We can send a user-provided stream, or use our internal buffer stream, but not both
		if (_buffer != _stream) {
			/*
			 * Because we manage both _buffer and _stream internally this should
			 * never happen. If it doesn, there's a problem somewhere in this class.
			 */
			debug_e("TcpClient::send ERROR buffer doesn't match stream");
			debug_hex(ERR, "send data", data, len);
			return false;
		}
	}
	else {
		auto buf = new MemoryDataStream();
		send(buf, forceCloseAfterSent);
		_buffer = buf;
	}

	if (_buffer->write((const uint8_t*)data, len) != len) {
		debug_e("TcpClient::send ERROR: Unable to store %d bytes in buffer", len);
		return false;
	}

	//	debug_d("Storing %d bytes in stream", len);
	debug_hex(DBG, "TcpClient::send", data, len);

	_asyncTotalLen += len;
	_asyncCloseAfterSent = forceCloseAfterSent;

	return true;
}

err_t TcpClient::onConnected(err_t err)
{
	if (err == ERR_OK)
		_state = eTCS_Connected;
	else
		onError(err);

	// Fire ReadyToSend callback
	TcpConnection::onConnected(err);

	return ERR_OK;
}

err_t TcpClient::onReceive(pbuf* buf)
{
	// Disconnected? close it
	if (!buf)
		return TcpConnection::onReceive(buf);

	if (_receive)
		for (pbuf* cur = buf; cur && cur->len > 0; cur = cur->next) {
			bool success = !_receive(*this, (char*)cur->payload, cur->len);
			if (!success) {
				debug_d("TcpClient::onReceive: Aborted from receive callback");

				TcpConnection::onReceive(nullptr);
				return ERR_ABRT; // abort the connection
			}
		}

	// Fire ReadyToSend callback
	TcpConnection::onReceive(buf);

	return ERR_OK;
}

void TcpClient::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	TcpConnection::onReadyToSendData(sourceEvent);
	if (_ready)
		_ready(*this, sourceEvent);

	if (!_stream)
		return;

	write(_stream);

	if (_stream->isFinished()) {
		flush();
		debug_d("TcpClient stream finished");
		freeStreams();
	}
}

void TcpClient::close()
{
	if (_state != eTCS_Successful && _state != eTCS_Failed) {
		_state = (_asyncTotalSent == _asyncTotalLen) ? eTCS_Successful : eTCS_Failed;
#ifdef ENABLE_SSL
		if (_ssl && _sslConnected)
			_state = (_asyncTotalLen == 0 || (_asyncTotalSent > _asyncTotalLen)) ? eTCS_Successful : eTCS_Failed;
#endif
		_asyncTotalLen = 0;
		_asyncTotalSent = 0;
		onFinished(_state);
	}

	// Close connection only after processing
	TcpConnection::close();
}

err_t TcpClient::onSent(uint16_t len)
{
	_asyncTotalSent += len;

	if (!_stream && _asyncCloseAfterSent) {
		TcpConnection::onSent(len);
		close();
	}
	else {
		// Fire ReadyToSend callback
		TcpConnection::onSent(len);
	}

	return ERR_OK;
}

void TcpClient::onError(err_t err)
{
	_state = eTCS_Failed;
	onFinished(_state);

	TcpConnection::onError(err);
}

void TcpClient::onFinished(TcpClientState finishState)
{
	freeStreams();

	// Initialize async variables for next connection
	_asyncTotalSent = 0;
	_asyncTotalLen = 0;

	if (_completed)
		_completed(*this, _state == eTCS_Successful);
}

#ifdef ENABLE_SSL
err_t TcpClient::onSslConnected(SSL* ssl)
{
	// No validators so accept connection
	if (_sslValidators.count() == 0)
		return ERR_OK;

	for (unsigned i = 0; i < _sslValidators.count(); i++)
		if (_sslValidators[i](ssl, _sslValidatorsData[i]))
			return ERR_OK;

	// No validation
	return ERR_ABRT;
}

void TcpClient::addSslValidator(SslValidatorCallback callback, const void* data /* = nullptr */)
{
	_sslValidators.addElement(callback);
	_sslValidatorsData.addElement(data);
}

bool TcpClient::pinCertificate(const uint8_t* fingerprint, SslFingerprintType type)
{
	SslValidatorCallback callback;
	if (type == eSFT_CertSha1)
		callback = sslValidateCertificateSha1;
	else if (type == eSFT_PkSha256)
		callback = sslValidatePublicKeySha256;
	else {
		debug_d("Unsupported SSL certificate fingerprint type");
		delete[] fingerprint;
		return false;
	}

	addSslValidator(callback, fingerprint);
	return true;
}

bool TcpClient::pinCertificate(const SSLFingerprints& fingerprints)
{
	bool success = false;
	if (fingerprints.certSha1)
		success = pinCertificate(fingerprints.certSha1, eSFT_CertSha1);

	if (fingerprints.pkSha256)
		success = pinCertificate(fingerprints.pkSha256, eSFT_PkSha256);

	return success;
}

#endif
