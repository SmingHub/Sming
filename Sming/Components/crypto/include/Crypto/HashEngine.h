/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HashEngine.h
 *
 ****/

#pragma once

#include "HashApi.h"

namespace Crypto
{
/**
 * @brief Hash engines just call the C API so class can be constructed using a macro
 */
#define CRYPTO_HASH_ENGINE(class_, name_, hashsize_, blocksize_)                                           \
	class class_##Engine                                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		static constexpr const char* name = #name_;                                                                    \
		static constexpr size_t hashsize = hashsize_;                                                                  \
		static constexpr size_t blocksize = blocksize_;                                                                \
                                                                                                                       \
		void init()                                                                                                    \
		{                                                                                                              \
			CRYPTO_NAME(name_, init)(&ctx);                                                                            \
		}                                                                                                              \
                                                                                                                       \
		void update(const void* data, size_t size)                                                                     \
		{                                                                                                              \
			CRYPTO_NAME(name_, update)(&ctx, static_cast<const uint8_t*>(data), size);                                 \
		}                                                                                                              \
                                                                                                                       \
		void final(uint8_t* hash)                                                                                      \
		{                                                                                                              \
			CRYPTO_NAME(name_, final)(hash, &ctx);                                                                     \
		}                                                                                                              \
                                                                                                                       \
	private:                                                                                                           \
		CRYPTO_CTX(name_) ctx;                                                                                         \
	};

/*
 * Engine definitions
 */

CRYPTO_HASH_ENGINE(Md5, md5, MD5_SIZE, MD5_BLOCKSIZE);
CRYPTO_HASH_ENGINE(Sha1, sha1, SHA1_SIZE, SHA1_BLOCKSIZE);
CRYPTO_HASH_ENGINE(Sha224, sha224, SHA224_SIZE, SHA224_BLOCKSIZE);
CRYPTO_HASH_ENGINE(Sha256, sha256, SHA256_SIZE, SHA256_BLOCKSIZE);
CRYPTO_HASH_ENGINE(Sha384, sha384, SHA384_SIZE, SHA384_BLOCKSIZE);
CRYPTO_HASH_ENGINE(Sha512, sha512, SHA512_SIZE, SHA512_BLOCKSIZE);

} // namespace Crypto
