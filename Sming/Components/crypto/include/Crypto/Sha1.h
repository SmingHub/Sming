#pragma once

#include "HashContext.h"

namespace Crypto
{
class Sha1Engine
{
public:
	using Hash = HashValue<SHA1_SIZE>;

	void init()
	{
		SHA1_Init(&context);
	}

	void update(const void* data, size_t size)
	{
		SHA1_Update(&context, static_cast<const uint8_t*>(data), size);
	}

	void final(Hash& hash)
	{
		SHA1_Final(hash.data, &context);
	}

private:
	SHA1_CTX context;
};

using Sha1 = HashContext<Sha1Engine>;

} // namespace Crypto
