/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AxHash.h - Context wrappers for axTLS hash implementations (testing)
 *
 ****/

#pragma once

#include <Crypto/HashContext.h>
#include <axtls-8266/crypto/crypto.h>

namespace Crypto
{
namespace Ax
{
#define AX_HASH_ENGINE(class_, name_, hashsize_, statesize_, blocksize_)                                               \
	class class_##Engine                                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		static constexpr const char* name = "ax." #name_;                                                              \
		static constexpr size_t hashsize = hashsize_;                                                                  \
		static constexpr size_t statesize = statesize_;                                                                \
		static constexpr size_t blocksize = blocksize_;                                                                \
		using Hash = ByteArray<hashsize>;                                                                              \
                                                                                                                       \
		void init()                                                                                                    \
		{                                                                                                              \
			name_##_Init(&context);                                                                                    \
		}                                                                                                              \
                                                                                                                       \
		void update(const void* data, size_t size)                                                                     \
		{                                                                                                              \
			name_##_Update(&context, static_cast<const uint8_t*>(data), size);                                         \
		}                                                                                                              \
                                                                                                                       \
		void final(uint8_t* hash)                                                                                      \
		{                                                                                                              \
			name_##_Final(hash, &context);                                                                             \
		}                                                                                                              \
                                                                                                                       \
		uint64_t get_state(void* state)                                                                                \
		{                                                                                                              \
			return 0;                                                                                                  \
		}                                                                                                              \
                                                                                                                       \
		void set_state(const void* state, uint64_t count)                                                              \
		{                                                                                                              \
		}                                                                                                              \
                                                                                                                       \
	private:                                                                                                           \
		name_##_CTX context;                                                                                           \
	};

AX_HASH_ENGINE(Md5, MD5, MD5_SIZE, MD5_STATESIZE, MD5_BLOCKSIZE);
AX_HASH_ENGINE(Sha1, SHA1, SHA1_SIZE, SHA1_STATESIZE, SHA1_BLOCKSIZE);
AX_HASH_ENGINE(Sha256, SHA256, SHA256_SIZE, SHA256_STATESIZE, SHA256_BLOCKSIZE);
AX_HASH_ENGINE(Sha384, SHA384, SHA384_SIZE, SHA384_STATESIZE, SHA384_BLOCKSIZE);
AX_HASH_ENGINE(Sha512, SHA512, SHA512_SIZE, SHA512_STATESIZE, SHA512_BLOCKSIZE);

using Md5 = HashContext<Md5Engine>;
using Sha1 = HashContext<Sha1Engine>;
using Sha256 = HashContext<Sha256Engine>;
using Sha384 = HashContext<Sha384Engine>;
using Sha512 = HashContext<Sha512Engine>;

} // namespace Ax

} // namespace Crypto
