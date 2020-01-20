/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrServerConnection.cpp
 *
 * @author: 2019 - mikee47 <mike@sillyhouse.net>
 *
 ****/

/*
 */
#include <SslDebug.h>
#include "BrServerConnection.h"
#include <Network/Ssl/Session.h>

namespace Ssl
{
int BrServerConnection::init()
{
	br_ssl_server_zero(&serverContext);

	// Server requires bi-directional buffer
	size_t bufSize = maxBufferSizeToBytes(context.session.maxBufferSize);
	if(bufSize == 0) {
		bufSize = 4096;
	}
	// add on minimum size for output
	bufSize += 512U;

	int err = BrConnection::init(bufSize, true);
	if(err < 0) {
		return err;
	}

	auto engine = getEngine();

	br_ssl_engine_add_flags(engine, BR_OPT_NO_RENEGOTIATION);

	auto& keyCert = context.session.keyCert;
	cert.data = const_cast<uint8_t*>(keyCert.getCertificate());
	cert.data_len = keyCert.getCertificateLength();
	if(!key.decode(keyCert.getKey(), keyCert.getKeyLength())) {
		debug_e("Failed to decode keyCert");
		return -BR_ERR_BAD_PARAM;
	}
	br_ssl_server_set_single_rsa(&serverContext, &cert, 1, key, BR_KEYTYPE_RSA | BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN,
								 br_rsa_private_get_default(), br_rsa_pkcs1_sign_get_default());
	// Warning: Inconsistent return type: not an error code
	if(!br_ssl_server_reset(&serverContext)) {
		debug_e("br_ssl_client_reset failed");
		return getLastError();
	}

	return startHandshake();
}

} // namespace Ssl
