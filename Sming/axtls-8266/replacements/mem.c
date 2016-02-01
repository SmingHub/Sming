/*
 * Memory functions for ESP8266 taken from Arduino-Esp project

  WiFiClientSecure.cpp - Variant of WiFiClient with TLS support
  Copyright (c) 2015 Ivan Grokhotkov. All rights reserved.
  This file is part of the esp8266 core for Arduino environment.


  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/

#include <stdint.h>
#include "espinc/c_types_compatible.h"

// Those Espressif functions are needed
extern void *pvPortMalloc(size_t xWantedSize, const char *file, uint32 line);
extern void *pvPortRealloc(void* ptr, size_t xWantedSize, const char *file, uint32 line);
extern void vPortFree(void *ptr, const char *file, uint32 line);
extern void *ets_memset(void *s, int c, size_t n);

#include "mem_manager.h"

#define free os_free
#define malloc os_malloc
#define realloc os_realloc
#define memset ets_memset

#ifdef DEBUG_TLS_MEM
extern int m_printf(const char *fmt, ...);
#define DEBUG_TLS_MEM_PRINT(...) m_printf(__VA_ARGS__)
#else
#define DEBUG_TLS_MEM_PRINT(...)
#endif

void* ax_port_malloc(size_t size, const char* file, int line) {
    void* result = (void *)malloc(size);

    if (result == NULL) {
        DEBUG_TLS_MEM_PRINT("%s:%d malloc %d failed, left %d\r\n", file, line, size, system_get_free_heap_size());

        while(true){}
    }
    if (size >= 1024)
        DEBUG_TLS_MEM_PRINT("%s:%d malloc %d, left %d\r\n", file, line, size, system_get_free_heap_size());
    return result;
}

void* ax_port_calloc(size_t size, size_t count, const char* file, int line) {
    void* result = (void* )ax_port_malloc(size * count, file, line);
    memset(result, 0, size * count);
    return result;
}

void* ax_port_realloc(void* ptr, size_t size, const char* file, int line) {
    void* result = (void* )realloc(ptr, size);
    if (result == NULL) {
        DEBUG_TLS_MEM_PRINT("%s:%d realloc %d failed, left %d\r\n", file, line, size, system_get_free_heap_size());
        while(true){}
    }
    if (size >= 1024)
        DEBUG_TLS_MEM_PRINT("%s:%d realloc %d, left %d\r\n", file, line, size, system_get_free_heap_size());
    return result;
}

void ax_port_free(void* ptr) {
    free(ptr);
    uint32_t *p = (uint32_t*) ptr;
    size_t size = p[-3];
    if (size >= 1024)
        DEBUG_TLS_MEM_PRINT("free %d, left %d\r\n", p[-3], system_get_free_heap_size());
}
