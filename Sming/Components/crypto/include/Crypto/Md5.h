#pragma once

#include "HashContext.h"

namespace Crypto
{
using Md5Hash = HashValue<MD5_SIZE>;

class Md5 : public HashContext<Md5, Md5Hash>
{
public:
	using Hash = Md5Hash;

	Md5()
	{
		init();
	}

	void init()
	{
		MD5_Init(&context);
	}

	using HashContext::update;

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

} // namespace Crypto
