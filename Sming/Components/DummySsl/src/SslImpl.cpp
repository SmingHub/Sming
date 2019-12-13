/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxtlsContext.cpp
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include <Network/Ssl/SslInterface.h>
#include <Network/Ssl/SslCrypto.h>

extern "C" {

void hmac_md5(const uint8_t* msg, int length, const uint8_t* key, int key_len, uint8_t* digest)
{
	debug_e("DummySSL is not a real SSL implementation");
}

} // extern "C"

SslContext* sslCreateContext()
{
	debug_e("DummySSL is not a real SSL implementation");
	return nullptr;
}

SslExtension* sslCreateExtension()
{
	debug_e("DummySSL is not a real SSL implementation");
	return nullptr;
}
