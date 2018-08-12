#ifndef SERVICES_SPIFFS_SPIFFY_HOST_H_
#define SERVICES_SPIFFS_SPIFFY_HOST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#if defined(_WIN32) || defined(__CYGWIN__)
// For compatibility with different OS & configurations
#if __WORDSIZE == 64
#if !defined(__intptr_t_defined) && !defined(_INTPTR_T_DEFINED)
typedef long int intptr_t;
#define __intptr_t_defined
#define _INTPTR_T_DEFINED
#endif
#if !defined(__uintptr_t_defined) && !defined(_UINTPTR_T_DEFINED)
#define __uintptr_t_defined
#define _UINTPTR_T_DEFINED
#undef uintptr_t
typedef unsigned long int uintptr_t;
#endif /* _UINTPTR_T_DEFINED */
#else
#if !defined(__intptr_t_defined) && !defined(_INTPTR_T_DEFINED)
typedef int intptr_t;
#define __intptr_t_defined
#define _INTPTR_T_DEFINED
#endif
#if !defined(__uintptr_t_defined) && !defined(_UINTPTR_T_DEFINED)
#define __uintptr_t_defined
#define _UINTPTR_T_DEFINED
#undef uintptr_t
typedef unsigned int uintptr_t;
#endif /* _UINTPTR_T_DEFINED */
#endif
#else
#include <stdint.h>
#endif
typedef signed int s32_t;
typedef unsigned int u32_t;
typedef signed short s16_t;
typedef unsigned short u16_t;
typedef signed char s8_t;
typedef unsigned char u8_t;

#endif /* SERVICES_SPIFFS_SPIFFY_HOST_H_ */
