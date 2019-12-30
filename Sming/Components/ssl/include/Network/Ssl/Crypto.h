/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Crypto.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <stdint.h>

/**
 * @ingroup ssl
 * @{
 */

#define MD5_SIZE 16
#define SHA1_SIZE 20
#define SHA256_SIZE 32

// Common cryptographic support functions
#ifdef __cplusplus
extern "C" {
#endif

void hmac_md5(const uint8_t* msg, int length, const uint8_t* key, int key_len, uint8_t* digest);

#ifdef __cplusplus
}
#endif

/** @} */
