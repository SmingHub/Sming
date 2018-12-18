/*
 * Copyright (c) 2007-2015, Cameron Rich
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

//#include "../crypto/os_int.h"
#include "c_types.h"
#include "osapi.h"
#include <stdio.h>
#include "lwip/app/time.h"

#if 0
#define ssl_printf(fmt, args...) os_printf(fmt,## args)
#else
#define ssl_printf(fmt, args...)
#endif

#define STDCALL
#define EXP_FUNC

//struct timeval {
//  unsigned long  tv_sec;         /* seconds */
//  unsigned long  tv_usec;        /* and microseconds */
//};

#define tls_htons(x) ((uint16)((((x) & 0xff) << 8) | (((x) >> 8) & 0xff)))
#define tls_ntohs(x) tls_htons(x)
#define tls_htonl(_n) ((uint32)( (((_n) & 0xff) << 24) | (((_n) & 0xff00) << 8) | (((_n) >> 8)  & 0xff00) | (((_n) >> 24) & 0xff) ))
#define tls_ntohl(x) tls_htonl(x)

#ifndef be16toh
#define be16toh(x) ((uint16)tls_ntohs((uint16)(x)))
#endif

#ifndef htobe16
#define htobe16(x) ((uint16)tls_htons((uint16)(x)))
#endif

#ifndef be32toh
#define be32toh(x) ((uint32)tls_ntohl((uint32)(x)))
#endif

#ifndef htobe32
#define htobe32(x) ((uint32)tls_htonl((uint32)(x)))
#endif

#ifndef be64toh
static __inline__ uint64 be64toh(uint64 __x);
static __inline__ uint64 be64toh(uint64 __x) {return (((uint64)be32toh(__x & (uint64)0xFFFFFFFFULL)) << 32) | ((uint64)be32toh((__x & (uint64)0xFFFFFFFF00000000ULL) >> 32));}
#define be64toh(x) be64toh(x)
#endif

#ifndef htobe64
#define htobe64(x) be64toh(x)
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
