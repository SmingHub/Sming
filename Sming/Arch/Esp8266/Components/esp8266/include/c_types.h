/*
 *  Copyright (c) 2010 - 2011 Espressif System
 *
 */

// Overrides c_types.h for all SDK versions

#ifndef _C_TYPES_H_
#define _C_TYPES_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BOOL bool
#define TRUE true
#define FALSE false

typedef uint8_t         uint8;
typedef uint8_t         u8;
typedef int8_t          sint8;
typedef int8_t          int8;
typedef int8_t          s8;
typedef uint16_t        uint16;
typedef uint16_t        u16;
typedef int16_t         sint16;
typedef int16_t         s16;
typedef uint32_t        uint32;
typedef unsigned int    u_int;
typedef uint32_t        u32;
typedef int32_t         sint32;
typedef int32_t         s32;
typedef int32_t         int32;
typedef int64_t         sint64;
typedef uint64_t        uint64;
typedef uint64_t        u64;
typedef float           real32;
typedef double          real64;

#endif /* _C_TYPES_H_ */
