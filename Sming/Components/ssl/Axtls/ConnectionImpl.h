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

namespace Ssl
{
class ConnectionImpl : public Connection
{
public:
	ConnectionImpl(tcp_pcb* tcp) : tcp(tcp)
	{
	}

	~ConnectionImpl()
	{
		delete certificate;
	}

	void init(SSL* ssl)
	{
		this->ssl = ssl;
	}

	bool isHandshakeDone() override
	{
		return (ssl_handshake_status(ssl) == SSL_OK);
	}

	int read(pbuf* encrypted, pbuf*& decrypted) override;

	int write(const uint8_t* data, size_t length) override;

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

	const Certificate* getCertificate() const override
	{
		if(certificate == nullptr && ssl->x509_ctx != nullptr) {
			certificate = new CertificateImpl(ssl);
		}

		return certificate;
	}

public:
	// Called from axTLS
	int port_write(uint8_t* buf, uint16_t bytes_needed);
	int port_read(uint8_t* buf, int bytes_needed);

private:
	SSL* ssl;
	mutable CertificateImpl* certificate = nullptr;
	struct tcp_pcb* tcp = nullptr;
	struct pbuf* tcp_pbuf = nullptr;
	int pbuf_offset = 0;
};

} // namespace Ssl
