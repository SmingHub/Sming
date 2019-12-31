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

	// Server requires bi-directional buffer, add on minimum size for output
	auto fragSize = context.getSession().fragmentSize ?: eSEFS_4K;
	size_t bufSize = (256U << fragSize) + (BR_SSL_BUFSIZE_MONO - 16384U);
	bufSize += 512U + (BR_SSL_BUFSIZE_OUTPUT - 16384);

	int err = BrConnection::init(bufSize, true);
	if(err < 0) {
		return err;
	}

	auto engine = getEngine();

	br_ssl_engine_add_flags(engine, BR_OPT_NO_RENEGOTIATION);

	auto& keyCert = context.getSession().keyCert;
	cert.data = const_cast<uint8_t*>(keyCert.getCertificate());
	cert.data_len = keyCert.getCertificateLength();
	err = key.decode(keyCert.getKey(), keyCert.getKeyLength());
	if(err < 0) {
		return err;
	}
	br_ssl_server_set_single_rsa(&serverContext, &cert, 1, key, BR_KEYTYPE_RSA | BR_KEYTYPE_KEYX | BR_KEYTYPE_SIGN,
								 br_rsa_private_get_default(), br_rsa_pkcs1_sign_get_default());
	br_ssl_server_reset(&serverContext);

	return startHandshake();
}

} // namespace Ssl
