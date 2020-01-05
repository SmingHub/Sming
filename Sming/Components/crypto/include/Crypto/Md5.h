#pragma once

#include "HashContext.h"

namespace Crypto
{
class Md5Engine
{
public:
	using Hash = HashValue<MD5_SIZE>;

	void init()
	{
		MD5_Init(&context);
	}

	void update(const void* data, size_t size)
	{
		MD5_Update(&context, static_cast<const uint8_t*>(data), size);
	}

	void final(Hash& hash)
	{
		MD5_Final(hash.data, &context);
	}

private:
	MD5_CTX context;
};

using Md5 = HashContext<Md5Engine>;

} // namespace Crypto
