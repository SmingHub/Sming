/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * EspHash.h - Context wrappers for ESP hash implementations (testing)
 *
 ****/

#pragma once

#include <Crypto/HashContext.h>
#include <esp_crypto.h>

namespace Crypto
{
namespace Esp
{
#define ESP_HASH_ENGINE(class_, name_, hashsize_, statesize_, blocksize_)                                              \
	class class_##Engine                                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		static constexpr const char* name = "esp." #name_;                                                             \
		static constexpr size_t hashsize = hashsize_;                                                                  \
		static constexpr size_t statesize = statesize_;                                                                \
		static constexpr size_t blocksize = blocksize_;                                                                \
		using Hash = ByteArray<hashsize>;                                                                              \
                                                                                                                       \
		void init()                                                                                                    \
		{                                                                                                              \
			ESP_##name_##_Init(&context);                                                                              \
		}                                                                                                              \
                                                                                                                       \
		void update(const void* data, size_t size)                                                                     \
		{                                                                                                              \
			ESP_##name_##_Update(&context, static_cast<const uint8_t*>(data), size);                                   \
		}                                                                                                              \
                                                                                                                       \
		void final(uint8_t* hash)                                                                                      \
		{                                                                                                              \
			ESP_##name_##_Final(hash, &context);                                                                       \
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
		ESP_##name_##_CTX context;                                                                                     \
	};

ESP_HASH_ENGINE(Md5, MD5, MD5_SIZE, MD5_STATESIZE, MD5_BLOCKSIZE);
ESP_HASH_ENGINE(Sha1, SHA1, SHA1_SIZE, SHA1_STATESIZE, SHA1_BLOCKSIZE);

using Md5 = HashContext<Md5Engine>;
using Sha1 = HashContext<Sha1Engine>;

} // namespace Esp

} // namespace Crypto
