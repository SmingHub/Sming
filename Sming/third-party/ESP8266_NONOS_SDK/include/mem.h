/*
 * ESPRESSIF MIT License
 *
 * Copyright (c) 2016 <ESPRESSIF SYSTEMS (SHANGHAI) PTE LTD>
 *
 * Permission is hereby granted for use on ESPRESSIF SYSTEMS ESP8266 only, in which case,
 * it is free of charge, to any person obtaining a copy of this software and associated
 * documentation files (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef __MEM_H__
#define __MEM_H__

#include <stddef.h>

#include "c_types.h"

/* Note: check_memleak_debug_enable is a weak function inside SDK.
 * please copy following codes to user_main.c.
#include "mem.h"

bool ICACHE_FLASH_ATTR check_memleak_debug_enable(void)
{
    return MEMLEAK_DEBUG_ENABLE;
}
*/

void *pvPortMalloc (size_t sz, const char *, unsigned, bool);
void vPortFree (void *p, const char *, unsigned);
void *pvPortZalloc (size_t sz, const char *, unsigned);
void *pvPortRealloc (void *p, size_t n, const char *, unsigned);
void* pvPortCalloc(size_t count,size_t size,const char *,unsigned);
void* pvPortCallocIram(size_t count,size_t size,const char *,unsigned);
void *pvPortZallocIram (size_t sz, const char *, unsigned);

#ifndef MEMLEAK_DEBUG
#define MEMLEAK_DEBUG_ENABLE    0
#define os_free(s)        vPortFree(s, "", __LINE__)
#define os_malloc_iram(s) pvPortMalloc(s, "", __LINE__,true)
#define os_malloc_dram(s) pvPortMalloc(s, "", __LINE__,false)
#define os_calloc_iram(l, s)   pvPortCallocIram(l, s, "", __LINE__)
#define os_calloc_dram(l, s)   pvPortCalloc(l, s, "", __LINE__)
#define os_realloc(p, s)  pvPortRealloc(p, s, "", __LINE__)
#define os_zalloc_iram(s)      pvPortZallocIram(s, "", __LINE__)
#define os_zalloc_dram(s)      pvPortZalloc(s, "", __LINE__)
#else
#define MEMLEAK_DEBUG_ENABLE	1

#define os_free(s) \
do{\
	static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
    vPortFree(s, mem_debug_file, __LINE__);\
}while(0)

#define os_malloc_iram(s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortMalloc(s, mem_debug_file, __LINE__,true);	\
	})

#define os_malloc_dram(s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortMalloc(s, mem_debug_file, __LINE__,false);	\
	})
    
#define os_calloc_iram(l, s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortCallocIram(l, s, mem_debug_file, __LINE__);	\
	})

#define os_calloc_dram(l, s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortCalloc(l, s, mem_debug_file, __LINE__);	\
	})
    
#define os_realloc(p, s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortRealloc(p, s, mem_debug_file, __LINE__);	\
	})

#define os_zalloc_iram(s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortZallocIram(s, mem_debug_file, __LINE__);	\
	})

#define os_zalloc_dram(s)	\
	({	\
		static const char mem_debug_file[] ICACHE_RODATA_ATTR STORE_ATTR = __FILE__;	\
		pvPortZalloc(s, mem_debug_file, __LINE__);	\
	})
    
#endif

#ifdef MEM_DEFAULT_USE_DRAM
#define os_malloc           os_malloc_dram
#define os_zalloc           os_zalloc_dram
#define os_calloc           os_calloc_dram
#else
#define os_malloc           os_malloc_iram
#define os_zalloc           os_zalloc_iram
#define os_calloc           os_calloc_iram
#endif
#endif

