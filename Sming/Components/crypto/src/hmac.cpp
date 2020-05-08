/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * hmac.cpp
 *
 ****/

#include "../include/Crypto/Md5.h"
#include "../include/Crypto/Sha1.h"
#include "../include/Crypto/Sha2.h"

namespace
{
template <class Hash>
void hmacVT(const uint8_t** msg, int* msg_len, int count, const uint8_t* key, int key_len, uint8_t* digest)
{
	Crypto::HmacContext<Hash> hmac(Crypto::Secret(key, key_len));
	for(int i = 0; i < count; ++i) {
		hmac.update(msg[i], msg_len[i]);
	}
	auto hash = hmac.getHash();
	memcpy(digest, hash.data(), hash.size());
}

} // namespace

#ifndef USE_ESP_CRYPTO

CRYPTO_FUNC_HMAC_V(md5)
{
	hmacVT<Crypto::Md5>(msg, msg_len, count, key, key_len, digest);
}

CRYPTO_FUNC_HMAC_V(sha1)
{
	hmacVT<Crypto::Sha1>(msg, msg_len, count, key, key_len, digest);
}

#endif

CRYPTO_FUNC_HMAC_V(sha256)
{
	hmacVT<Crypto::Sha256>(msg, msg_len, count, key, key_len, digest);
}

CRYPTO_FUNC_HMAC_V(sha384)
{
	hmacVT<Crypto::Sha384>(msg, msg_len, count, key, key_len, digest);
}

CRYPTO_FUNC_HMAC_V(sha512)
{
	hmacVT<Crypto::Sha512>(msg, msg_len, count, key, key_len, digest);
}
