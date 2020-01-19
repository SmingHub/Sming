#include "../include/Crypto/HmacContext.h"
#include "util.h"

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
