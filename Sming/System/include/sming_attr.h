/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sming_attr.h - low-level attribute definitions common to all architectures
 *
 ****/

#pragma once

#ifndef __forceinline
#define __forceinline __attribute__((always_inline)) inline
#endif

#ifndef __noinline
#define __noinline __attribute__((noinline))
#endif

// Weak attributes don't work for PE
#ifdef __WIN32
#define WEAK_ATTR
#else
#define WEAK_ATTR __attribute((weak))
#endif

/*
 * Use this definition in the cases where a function or a variable is meant to be possibly unused. GCC will not produce a warning for it.
 * NOTE: Use `[[maybe_unused]]` attribute in C++ code
 */
#define SMING_UNUSED __attribute__((unused))

/*
 * Flags a compiler warning when Sming framework methods, functions or types are changed
 */
#define SMING_DEPRECATED __attribute__((deprecated))
