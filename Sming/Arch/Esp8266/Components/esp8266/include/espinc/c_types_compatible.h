/*
 *  Copyright (c) 2010 - 2011 Espressif System
 *
 */

 // Updated, compatible version of c_types.h
 // Just removed types declared in <stdint.h>
 
#ifndef _SYSTEM_INCLUDE_ESPINC_C_TYPES_COMPATIBLE_H_
#define _SYSTEM_INCLUDE_ESPINC_C_TYPES_COMPATIBLE_H_

/*typedef unsigned char       uint8_t;
typedef signed char         sint8_t;
typedef signed char         int8_t;
typedef unsigned short      uint16_t;
typedef signed short        sint16_t;
typedef signed short        int16_t;
typedef unsigned long       uint32_t;
typedef signed long         sint32_t;
typedef signed long         int32_t;
typedef signed long long    sint64_t;
typedef unsigned long long  uint64_t;
typedef unsigned long long  u_int64_t;
typedef float               real32_t;
typedef double              real64_t;*/

typedef unsigned char       uint8;
typedef unsigned char       u8;
typedef signed char         sint8;
typedef signed char         int8;
typedef signed char         s8;
typedef unsigned short      uint16;
typedef unsigned short      u16;
typedef signed short        sint16;
typedef signed short        s16;
typedef unsigned int        uint32;
typedef unsigned int        u_int;
typedef unsigned int        u32;
typedef signed int          sint32;
typedef signed int          s32;
typedef int                 int32;
typedef signed long long    sint64;
typedef unsigned long long  uint64;
typedef unsigned long long  u64;
typedef float               real32;
typedef double              real64;

/* Additional type names */
typedef unsigned char       u8_t;
typedef unsigned short      u16_t;
typedef unsigned long       u32_t;

typedef signed char         s8_t;
typedef signed short        s16_t;
typedef signed long         s32_t;

#define __le16      u16

typedef unsigned int        size_t;

#define __packed        __attribute__((packed))

#define LOCAL       static

#ifndef NULL
#define NULL (void *)0
#endif /* NULL */

/* probably should not put STATUS here */
typedef enum {
    OK = 0,
    FAIL,
    PENDING,
    BUSY,
    CANCEL,
} STATUS;

#define BIT(nr)                 (1UL << (nr))

#define REG_SET_BIT(_r, _b)  (*(volatile uint32_t*)(_r) |= (_b))
#define REG_CLR_BIT(_r, _b)  (*(volatile uint32_t*)(_r) &= ~(_b))

#define DMEM_ATTR __attribute__((section(".bss")))
#define SHMEM_ATTR

#ifdef ICACHE_FLASH
#define ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#define ICACHE_RODATA_ATTR __attribute__((section(".irom.text")))
#else
#define ICACHE_FLASH_ATTR
#endif /* ICACHE_FLASH */

#ifndef __cplusplus
typedef unsigned char   bool;
#define BOOL            bool
#define true            ((bool)1)
#define false           ((bool)0)
#define TRUE            true
#define FALSE           false


#endif /* !__cplusplus */

#endif /* _SYSTEM_INCLUDE_ESPINC_C_TYPES_COMPATIBLE_H_ */
