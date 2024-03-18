/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxConnection.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Connection.h>
#include <Network/Ssl/Validator.h>
#include "AxCertificate.h"
#include "AxError.h"
#include <memory>

namespace Ssl
{
class AxConnection : public Connection
{
public:
	using Connection::Connection;

	~AxConnection()
	{
		// Typically sends out closing message
		ssl_free(ssl);
	}

	void init(SSL* ssl)
	{
		this->ssl = ssl;
	}

	bool isHandshakeDone() const override
	{
		return (ssl_handshake_status(ssl) == SSL_OK);
	}

	int write(const uint8_t* data, size_t length) override;

	CipherSuite getCipherSuite() const override
	{
		return CipherSuite(ssl_get_cipher_id(ssl));
	}

	SessionId getSessionId() const override
	{
		SessionId id;
		if(isHandshakeDone()) {
			id.assign(ssl->session_id, ssl->sess_id_size);
		}

		return id;
	}

	const Certificate* getCertificate() const override
	{
		if(!certificate && ssl->x509_ctx != nullptr) {
			certificate = std::make_unique<AxCertificate>(ssl);
		}

		return certificate.get();
	}

	void freeCertificate() override
	{
		certificate.reset();
	}

	int read(InputBuffer& input, uint8_t*& output) override;

	size_t readTcpData(uint8_t* buf, size_t count)
	{
		assert(input != nullptr);
		return input->read(buf, count);
	}

	String getErrorString(int error) const override
	{
		return Ssl::getErrorString(error);
	}

	Alert getAlert(int error) const override
	{
		return Ssl::getAlert(error);
	}

private:
	SSL* ssl{nullptr};
	mutable std::unique_ptr<AxCertificate> certificate;
	InputBuffer* input{nullptr};
};

} // namespace Ssl
