/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrClientConnection.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

/*
 */
#include <SslDebug.h>
#include "BrClientConnection.h"
#include <Network/Ssl/Session.h>

namespace Ssl
{
int BrClientConnection::init()
{
	br_ssl_client_zero(&clientContext);

	// Use Mono-directional buffer size according to requested max. fragment size
	size_t bufSize = maxBufferSizeToBytes(context.getSession().maxBufferSize);
	if(bufSize == 0) {
		bufSize = 4096;
	}
	int err = BrConnection::init(bufSize, false);
	if(err < 0) {
		return err;
	}

	br_ssl_client_set_default_rsapub(&clientContext);

	// X509 verification
	delete certificate;
	certificate = new BrCertificate([this]() { return context.getSession().validateCertificate(); });
	br_ssl_engine_set_x509(getEngine(), *certificate);

	br_ssl_client_reset(&clientContext, context.getSession().hostName.c_str(), 0);

	return startHandshake();
}

} // namespace Ssl
