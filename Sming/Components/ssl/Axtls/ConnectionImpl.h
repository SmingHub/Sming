/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ConnectionImpl.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/Connection.h>
#include "CertificateImpl.h"
#include <axtls-8266/compat/lwipr_compat.h>

namespace Ssl
{
class ConnectionImpl : public Connection
{
public:
	ConnectionImpl(SSL* ssl) : ssl(ssl), certificate(ssl)
	{
	}

	bool isHandshakeDone() override
	{
		return (ssl_handshake_status(ssl) == SSL_OK);
	}

	int read(tcp_pcb* tcp, pbuf* encrypted, pbuf*& decrypted) override
	{
		int read_bytes = axl_ssl_read(ssl, tcp, encrypted, &decrypted);

		// TODO: process the response and check if it connection abort or an error....

		return read_bytes;
	}

	int write(const uint8_t* data, size_t length) override
	{
		return axl_ssl_write(ssl, data, length);
	}

	int calcWriteSize(size_t plainTextLength) const override
	{
		return ssl_calculate_write_length(ssl, plainTextLength);
	}

	CipherSuite getCipherSuite() const override
	{
		return CipherSuite(ssl_get_cipher_id(ssl));
	}

	SessionId getSessionId() const override
	{
		SessionId id;
		if(ssl_handshake_status(ssl) == SSL_OK) {
			id.assign(ssl->session_id, ssl->sess_id_size);
		}

		return id;
	}

	const Certificate& getCertificate() const override
	{
		return certificate;
	}

private:
	SSL* ssl;
	CertificateImpl certificate;
	SessionId sessionId;
};

} // namespace Ssl
