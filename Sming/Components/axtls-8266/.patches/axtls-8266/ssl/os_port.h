/*
 * Copyright (c) 2007-2016, Cameron Rich
 * 
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * @file os_port.h
 *
 * Some stuff to minimise the differences between windows and linux/unix
 */

#ifndef HEADER_OS_PORT_H
#define HEADER_OS_PORT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os_int.h"
#include "config.h"
#include <FakePgmSpace.h>

#undef WIN32
#ifndef ESP8266
#define ESP8266
#endif

#define STDCALL
#define EXP_FUNC

#include "../util/time.h"
#include <errno.h>
#define TTY_FLUSH()
#undef putc

#define SOCKET_READ(A,B,C)      ax_port_read(A,B,C)
#define SOCKET_WRITE(A,B,C)     ax_port_write(A,B,C)
#define SOCKET_CLOSE(A)         ax_port_close(A)
#define get_file                ax_get_file
#define EWOULDBLOCK EAGAIN

#ifndef be64toh
# define __bswap_constant_64(x) \
     ((((x) & 0xff00000000000000ull) >> 56)                                   \
      | (((x) & 0x00ff000000000000ull) >> 40)                                 \
      | (((x) & 0x0000ff0000000000ull) >> 24)                                 \
      | (((x) & 0x000000ff00000000ull) >> 8)                                  \
      | (((x) & 0x00000000ff000000ull) << 8)                                  \
      | (((x) & 0x0000000000ff0000ull) << 24)                                 \
      | (((x) & 0x000000000000ff00ull) << 40)                                 \
      | (((x) & 0x00000000000000ffull) << 56))
#define be64toh(x) __bswap_constant_64(x)
#endif

extern void system_soft_wdt_feed(void);
#define ax_wdt_feed system_soft_wdt_feed

#define ax_array_read_u8(x, y) pgm_read_byte((x)+(y))

#ifdef AXTLS_BUILD

#define get_random(num_rand_bytes, rand_data) os_get_random(rand_data, num_rand_bytes)

#define printf(fmt, ...) m_printf(_F(fmt), ##__VA_ARGS__)

#undef strcpy_P
#define strcpy_P(a, str) strcpy(a, _F(str))

#endif /* AXTLS_BUILD */

// Implemented outside this library
extern int ax_port_read(int fd, uint8_t* buffer, int count);
extern int ax_port_write(int fd, uint8_t* buffer, uint16_t count);

// Not ANSI C so prototype required
extern char *strdup(const char *orig);

/* some functions to mutate the way these work */
#ifndef ntohl
static inline uint32_t htonl(uint32_t n){
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

#define ntohl htonl
#define LWIP_DONT_PROVIDE_BYTEORDER_FUNCTIONS
#endif

/* Mutexing definitions */
#define SSL_CTX_MUTEX_INIT(A)
#define SSL_CTX_MUTEX_DESTROY(A)
#define SSL_CTX_LOCK(A)
#define SSL_CTX_UNLOCK(A)

#ifdef __cplusplus
}
#endif

#endif
