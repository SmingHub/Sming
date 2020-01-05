#pragma once

#include "HashContext.h"

#ifdef ARCH_ESP8266
#include <esp_crypto.h>
#define CRYPTO_DEF(f) ESP_SHA1_##f
#else
#define CRYPTO_DEF(f) SHA1_##f
#endif

namespace Crypto
{
class Sha1Engine
{
public:
	using Hash = HashValue<SHA1_SIZE>;

	void init()
	{
		CRYPTO_DEF(Init)(&context);
	}

	void update(const void* data, size_t size)
	{
		CRYPTO_DEF(Update)(&context, static_cast<const uint8_t*>(data), size);
	}

	void final(Hash& hash)
	{
		CRYPTO_DEF(Final)(hash.data, &context);
	}

private:
	CRYPTO_DEF(CTX) context;
};

#undef CRYPTO_DEF

using Sha1 = HashContext<Sha1Engine>;

} // namespace Crypto
