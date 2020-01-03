#pragma once

#include "HashContext.h"

namespace Crypto
{
using Sha1Hash = HashValue<SHA1_SIZE>;

class Sha1 : public HashContext<Sha1, Sha1Hash>
{
public:
	using Hash = Sha1Hash;

	Sha1()
	{
		init();
	}

	void init()
	{
		SHA1_Init(&context);
	}

	void update(const void* data, size_t size)
	{
		SHA1_Update(&context, static_cast<const uint8_t*>(data), size);
	}

	using HashContext::update;

	void final(Hash& hash)
	{
		SHA1_Final(hash.data, &context);
	}

private:
	SHA1_CTX context;
};

} // namespace Crypto
