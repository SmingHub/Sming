/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * SslDebug.h
 *
 * #include this file ahead of others to enable informational and debug messages only when SSL_DEBUG is defined.
 *
 ****/

#pragma once

#if !defined(SSL_DEBUG) && (DEBUG_VERBOSE_LEVEL >= 2)
#undef DEBUG_VERBOSE_LEVEL
#define DEBUG_VERBOSE_LEVEL 1
#endif

#include <debug_progmem.h>
#include <assert.h>
