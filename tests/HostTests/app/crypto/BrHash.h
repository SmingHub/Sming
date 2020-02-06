/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BrHash.h - Context wrappers for BearSSL hash implementations (testing)
 *
 ****/

#pragma once

#include <Crypto/HashContext.h>
#include <bearssl_hash.h>

namespace Crypto
{
namespace Br
{
#define BR_HASH_ENGINE(class_, name_, hashsize_, statesize_, blocksize_)                                               \
	class class_##Engine                                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		static constexpr const char* name = "br." #name_;                                                              \
		static constexpr size_t hashsize = hashsize_;                                                                  \
		static constexpr size_t statesize = statesize_;                                                                \
		static constexpr size_t blocksize = blocksize_;                                                                \
		using Hash = ByteArray<hashsize>;                                                                              \
                                                                                                                       \
		void init()                                                                                                    \
		{                                                                                                              \
			br_##name_##_init(&context);                                                                               \
		}                                                                                                              \
                                                                                                                       \
		void update(const void* data, size_t size)                                                                     \
		{                                                                                                              \
			br_##name_##_update(&context, data, size);                                                                 \
		}                                                                                                              \
                                                                                                                       \
		void final(uint8_t* hash)                                                                                      \
		{                                                                                                              \
			br_##name_##_out(&context, hash);                                                                          \
		}                                                                                                              \
                                                                                                                       \
		uint64_t get_state(void* state)                                                                                \
		{                                                                                                              \
			return br_##name_##_state(&context, state);                                                                \
		}                                                                                                              \
                                                                                                                       \
		void set_state(const void* state, uint64_t count)                                                              \
		{                                                                                                              \
			br_##name_##_set_state(&context, state, count);                                                            \
		}                                                                                                              \
                                                                                                                       \
	private:                                                                                                           \
		br_##name_##_context context;                                                                                  \
	};

BR_HASH_ENGINE(Md5, md5, MD5_SIZE, MD5_STATESIZE, MD5_BLOCKSIZE);
BR_HASH_ENGINE(Sha1, sha1, SHA1_SIZE, SHA1_STATESIZE, SHA1_BLOCKSIZE);
BR_HASH_ENGINE(Sha224, sha224, SHA224_SIZE, SHA224_STATESIZE, SHA224_BLOCKSIZE);
BR_HASH_ENGINE(Sha256, sha256, SHA256_SIZE, SHA256_STATESIZE, SHA256_BLOCKSIZE);
BR_HASH_ENGINE(Sha384, sha384, SHA384_SIZE, SHA384_STATESIZE, SHA384_BLOCKSIZE);
BR_HASH_ENGINE(Sha512, sha512, SHA512_SIZE, SHA512_STATESIZE, SHA512_BLOCKSIZE);

using Md5 = HashContext<Md5Engine>;
using Sha1 = HashContext<Sha1Engine>;
using Sha224 = HashContext<Sha224Engine>;
using Sha256 = HashContext<Sha256Engine>;
using Sha384 = HashContext<Sha384Engine>;
using Sha512 = HashContext<Sha512Engine>;

} // namespace Br

} // namespace Crypto
