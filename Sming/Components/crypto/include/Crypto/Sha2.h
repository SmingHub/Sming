/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Sha2.h
 *
 ****/

#pragma once

#include "HashApi/sha2.h"
#include "HashEngine.h"
#include "HashContext.h"
#include "HmacContext.h"

namespace Crypto
{
CRYPTO_HASH_ENGINE_STD(Sha224, sha224, SHA224_SIZE, SHA224_STATESIZE, SHA224_BLOCKSIZE);
CRYPTO_HASH_ENGINE_STD(Sha256, sha256, SHA256_SIZE, SHA256_STATESIZE, SHA256_BLOCKSIZE);
CRYPTO_HASH_ENGINE_STD(Sha384, sha384, SHA384_SIZE, SHA384_STATESIZE, SHA384_BLOCKSIZE);
CRYPTO_HASH_ENGINE_STD(Sha512, sha512, SHA512_SIZE, SHA512_STATESIZE, SHA512_BLOCKSIZE);

/*
 * Hash contexts
 */

using Sha224 = HashContext<Sha224Engine>;
using Sha256 = HashContext<Sha256Engine>;
using Sha384 = HashContext<Sha384Engine>;
using Sha512 = HashContext<Sha512Engine>;

/*
 * HMAC contexts
 */

using HmacSha224 = HmacContext<Sha224>;
using HmacSha256 = HmacContext<Sha256>;
using HmacSha384 = HmacContext<Sha384>;
using HmacSha512 = HmacContext<Sha512>;

} // namespace Crypto
