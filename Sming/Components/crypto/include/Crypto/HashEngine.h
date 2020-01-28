/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HashEngine.h - Hash engine template code
 *
 ****/

#pragma once

/**
 * @brief Macro template to construct Engine class wrapper for a given hash API
 * @param class_ Name for the class type, upper-case camel
 * @param name_ Name for the C API, lower-case camel
 * @param hashsize_ Size of hash in bytes
 * @param statesize_ Size of state in bytes
 * @param blocksize_ Size of block buffer in bytes
 * @param INIT Macro to implement init/state and any other custom interface code
 *
 * Init methods may have optional parameters.
 * State methods are only required for standard hashes. These are used by Bear SSL multi-hash implementation.
 */
#define CRYPTO_HASH_ENGINE(class_, name_, hashsize_, statesize_, blocksize_, INIT)                                     \
	class class_##Engine                                                                                               \
	{                                                                                                                  \
	public:                                                                                                            \
		static constexpr const char* name = #name_;                                                                    \
		static constexpr size_t hashsize = hashsize_;                                                                  \
		static constexpr size_t statesize = statesize_;                                                                \
		static constexpr size_t blocksize = blocksize_;                                                                \
                                                                                                                       \
		INIT(name_)                                                                                                    \
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

/**
 * @brief Macro template to construct standard hash engines
 */
#define CRYPTO_HASH_ENGINE_STD(class_, name_, hashsize_, statesize_, blocksize_)                                       \
	CRYPTO_HASH_ENGINE(class_, name_, hashsize_, statesize_, blocksize_, CRYPTO_HASH_ENGINE_STD_INIT)

/**
 * @brief Macro template to provide init/state methods for standard hash engines
 */
#define CRYPTO_HASH_ENGINE_STD_INIT(name_)                                                                             \
	void init()                                                                                                        \
	{                                                                                                                  \
		CRYPTO_NAME(name_, init)(&ctx);                                                                                \
	}                                                                                                                  \
                                                                                                                       \
	uint64_t get_state(void* state)                                                                                    \
	{                                                                                                                  \
		return CRYPTO_NAME(name_, get_state)(&ctx, state);                                                             \
	}                                                                                                                  \
                                                                                                                       \
	void set_state(const void* state, uint64_t count)                                                                  \
	{                                                                                                                  \
		CRYPTO_NAME(name_, set_state)(&ctx, state, count);                                                             \
	}
