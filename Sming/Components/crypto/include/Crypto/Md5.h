#pragma once

#include "HashContext.h"

#ifdef ARCH_ESP8266
#include <esp_crypto.h>
#define CRYPTO_DEF(f) ESP_MD5_##f
#else
#define CRYPTO_DEF(f) MD5_##f
#endif

namespace Crypto
{
class Md5Engine
{
public:
	using Hash = HashValue<Md5Engine>;
	static constexpr const char* name = "MD5";
	static constexpr size_t size = MD5_SIZE;

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

using Md5 = HashContext<Md5Engine>;

} // namespace Crypto
