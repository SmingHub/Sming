/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsConnection.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "AxtlsConnection.h"
#include "AxtlsCertificate.h"

int AxtlsConnection::read(tcp_pcb* tcp, pbuf* encrypted, pbuf** decrypted) {

	int read_bytes = axl_ssl_read(ssl, tcp, encrypted, decrypted);

	// TODO: process the response and check if it connection abort or an error....

	return read_bytes;
}

const String AxtlsConnection::getCipher() const
{
	switch (ssl_get_cipher_id(ssl)) {
	case SSL_AES128_SHA:
		return String("AES128-SHA");

	case SSL_AES256_SHA:
		return String("AES256-SHA");

	case SSL_AES128_SHA256:
		return String("SSL_AES128_SHA256");

	case SSL_AES256_SHA256:
		return String("SSL_AES256_SHA256");

	default:
		return String("Unknown - %u", ssl_get_cipher_id(ssl));
	}
}

SslSessionId* AxtlsConnection::getSessionId()
{
	if(sessionId == nullptr) {
		sessionId = new SslSessionId();
	}

	if(ssl_handshake_status(ssl) == SSL_OK) {
		sessionId->assign(ssl->session_id, ssl->sess_id_size);
	}

	return sessionId;
}

SslCertificate* AxtlsConnection::getCertificate()
{
	if(certificate == nullptr) {
		certificate = new AxtlsCertificate(ssl);
	}

	return certificate;
}
