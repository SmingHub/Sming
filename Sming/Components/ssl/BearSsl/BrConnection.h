/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrConnection.h
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

#pragma once

#include <Network/Ssl/Connection.h>
#include "BrError.h"
#include "BrCertificate.h"
#include <bearssl.h>

namespace Ssl
{
template <typename T> void freeAndNil(T*& objectPointer)
{
	delete objectPointer;
	objectPointer = nullptr;
}

class BrConnection : public Connection
{
public:
	using Connection::Connection;

	~BrConnection()
	{
		delete[] buffer;
	}

	int read(InputBuffer& input, uint8_t*& output) override;

	int write(const uint8_t* data, size_t length) override;

	CipherSuite getCipherSuite() const override
	{
		if(handshakeDone) {
			return CipherSuite(getEngine()->session.cipher_suite);
		} else {
			return CipherSuite::NULL_WITH_NULL_NULL;
		}
	}

	SessionId getSessionId() const override
	{
		SessionId id;
		if(handshakeDone) {
			auto& param = getEngine()->session;
			id.assign(param.session_id, param.session_id_len);
		}

		return id;
	}

	bool isHandshakeDone() const override
	{
		return handshakeDone;
	}

	String getErrorString(int error) const override
	{
		return Ssl::getErrorString(error);
	}

	Alert getAlert(int error) const override
	{
		return Ssl::getAlert(error);
	}

	virtual br_ssl_engine_context* getEngine() = 0;

	br_ssl_engine_context* getEngine() const
	{
		return const_cast<BrConnection*>(this)->getEngine();
	}

	int getLastError() const
	{
		return -br_ssl_engine_last_error(getEngine());
	}

protected:
	/**
	 * Perform initialisation common to both client and server connections
	 * @param bufferSize Buffer to allocate excluding overheads
	 * @param bidi Whether to use bi-directional buffering (true) or mono
	 */
	int init(size_t bufferSize, bool bidi);

	int runUntil(InputBuffer& input, unsigned target);

	int startHandshake()
	{
		InputBuffer input(nullptr);
		return runUntil(input, BR_SSL_SENDAPP | BR_SSL_RECVAPP);
	}

private:
	void setCipherSuites(const CipherSuites::Array* cipherSuites);

private:
	uint8_t* buffer = nullptr;
	bool handshakeDone = false;
};

} // namespace Ssl
