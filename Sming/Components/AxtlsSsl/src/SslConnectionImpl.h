/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsConnection.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Network/Ssl/SslInterface.h>
#include <axtls-8266/compat/lwipr_compat.h>
#include "SslCertificateImpl.h"

class SslConnectionImpl : public SslConnection
{
public:
	SslConnectionImpl(SSL* ssl) : ssl(ssl)
	{
	}

	bool isHandshakeDone() override
	{
		return (ssl_handshake_status(ssl) == SSL_OK);
	}

	int read(tcp_pcb* tcp, pbuf* encrypted, pbuf** decrypted) override;

	int write(const uint8_t* data, size_t length) override
	{
		return axl_ssl_write(ssl, data, length);
	}

	int calcWriteSize(size_t plainTextLength) override
	{
		return ssl_calculate_write_length(ssl, plainTextLength);
	}

	const String getCipher() const override;

	SslSessionId* getSessionId() override;

	SslCertificate* getCertificate() override;

	virtual ~SslConnectionImpl()
	{
		delete certificate;
		delete sessionId;
		certificate = nullptr;
		sessionId = nullptr;
	}

protected:
	SslContext* context = nullptr;
	SSL* ssl = nullptr;
	SslCertificateImpl* certificate = nullptr; // << The certificate object is owned by this class
	SslSessionId* sessionId = nullptr;		   // << The sslSessionId object is owned by this class
};
