/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Sha1.h
 *
 ****/

#pragma once

#include "HashApi/sha1.h"
#include "HashEngine.h"
#include "HashContext.h"
#include "HmacContext.h"

namespace Crypto
{
CRYPTO_HASH_ENGINE_STD(Sha1, sha1, SHA1_SIZE, SHA1_STATESIZE, SHA1_BLOCKSIZE);

using Sha1 = HashContext<Sha1Engine>;

using HmacSha1 = HmacContext<Sha1>;

} // namespace Crypto
