/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#include <stdint.h>

#ifndef _INTEGER
#define _INTEGER

/* These types must be 16-bit, 32-bit or larger integer */
typedef int32_t			INT;
typedef uint32_t	    UINT;

/* These types must be 8-bit integer */
typedef char			CHAR;
typedef unsigned char	UCHAR;
typedef uint8_t	        BYTE;

/* These types must be 16-bit integer */
typedef int16_t			SHORT;
typedef uint16_t        USHORT;
typedef uint16_t	    WORD;
typedef uint16_t	    WCHAR;

/* These types must be 32-bit integer */
typedef int32_t			LONG;
typedef uint32_t	    ULONG;
typedef uint32_t	    DWORD;

#endif
