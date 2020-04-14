/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Md5.h
 *
 ****/

#pragma once

#include "HashApi/md5.h"
#include "HashEngine.h"
#include "HashContext.h"
#include "HmacContext.h"

namespace Crypto
{
CRYPTO_HASH_ENGINE_STD(Md5, md5, MD5_SIZE, MD5_STATESIZE, MD5_BLOCKSIZE);

using Md5 = HashContext<Md5Engine>;

using HmacMd5 = HmacContext<Md5>;

} // namespace Crypto
