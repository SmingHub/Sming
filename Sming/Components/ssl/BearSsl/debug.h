#pragma once

#if !defined(SSL_DEBUG) && (DEBUG_VERBOSE_LEVEL >= 2)
#undef DEBUG_VERBOSE_LEVEL
#define DEBUG_VERBOSE_LEVEL 1
#endif

#include <debug_progmem.h>
#include <assert.h>
