#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#pragma GCC warning "Use of 'user_config.h' is deprecated - see https://sming.readthedocs.io/en/latest/upgrading/4.4-4.5.html"

#include <esp_systemapi.h>

// Extended string conversion for compatibility
#include <stringconversion.h>

typedef unsigned char u8_t;
typedef signed char s8_t;
typedef unsigned short u16_t;
typedef signed short s16_t;
typedef unsigned long u32_t;
typedef signed long s32_t;
typedef unsigned long mem_ptr_t;
typedef signed short sint16_t;

#endif
