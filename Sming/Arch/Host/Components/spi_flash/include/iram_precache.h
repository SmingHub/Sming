#pragma once

#define IRAM_PRECACHE_ATTR
#define IRAM_PRECACHE_START(tag)
#define IRAM_PRECACHE_END(tag)

#ifdef __cplusplus
extern "C" {
#endif

static inline void iram_precache(void* addr, uint32_t bytes)
{
}

#ifdef __cplusplus
}
#endif
