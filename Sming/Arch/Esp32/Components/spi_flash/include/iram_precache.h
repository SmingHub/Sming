/*
 * Dummy stub. ESP32 has tons of IRAM.
*/

#include <stdint.h>

#pragma once

/**
 * @defgroup iram_precache IRAM Precache Support
 * @ingroup flash
 * @{
 */

#define IRAM_PRECACHE_ATTR
#define IRAM_PRECACHE_START(tag)
#define IRAM_PRECACHE_END(tag)

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Pre-load flash data into the flash instruction cache
 *  @param addr First location to cache, specify NULL to use current location.
 *  @param bytes Number of bytes to cache
 *  @note All pages containing the requested region will be read to pull them into cache RAM.
 */
inline void iram_precache(void* addr, uint32_t bytes)
{
}

/** @} */

#ifdef __cplusplus
}
#endif
