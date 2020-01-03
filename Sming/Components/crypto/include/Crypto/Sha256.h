#pragma once

#include "HashContext.h"

namespace Crypto
{
using Sha256Hash = HashValue<SHA256_SIZE>;

class Sha256 : public HashContext<Sha256, Sha256Hash>
{
public:
	using Hash = Sha256Hash;

	Sha256()
	{
		init();
	}

	void init()
	{
		SHA256_Init(&context);
	}

	using HashContext::update;

	void update(const void* data, size_t size)
	{
		SHA256_Update(&context, static_cast<const uint8_t*>(data), size);
	}

	void final(Hash& hash)
	{
		SHA256_Final(hash.data, &context);
	}

private:
	SHA256_CTX context;
};

} // namespace Crypto
