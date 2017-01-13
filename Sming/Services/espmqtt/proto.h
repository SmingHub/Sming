/* 
 * File:   proto.h
 * Author: ThuHien
 *
 * Created on November 23, 2012, 8:57 AM
 */

#ifndef _PROTO_H_
#define	_PROTO_H_
#include <stdlib.h>

#include "mqtt_typedef.h"
#include "ringbuf.h"

typedef void (PROTO_PARSE_CALLBACK)();

typedef struct
{
	uint8_t *buf;
	U16 bufSize;
	U16 dataLen;
	uint8_t isEsc;
	uint8_t isBegin;
	PROTO_PARSE_CALLBACK* callback;
} PROTO_PARSER;

I8 ICACHE_FLASH_ATTR PROTO_Init(PROTO_PARSER *parser,
		PROTO_PARSE_CALLBACK *completeCallback, uint8_t *buf, U16 bufSize);
I8 ICACHE_FLASH_ATTR PROTO_Parse(PROTO_PARSER *parser, uint8_t *buf, U16 len);
I16 ICACHE_FLASH_ATTR PROTO_Add(uint8_t *buf, const uint8_t *packet,
		I16 bufSize);
I16 ICACHE_FLASH_ATTR PROTO_AddRb(RINGBUF *rb, const uint8_t *packet, I16 len);
I8 ICACHE_FLASH_ATTR PROTO_ParseByte(PROTO_PARSER *parser, uint8_t value);
I16 ICACHE_FLASH_ATTR PROTO_ParseRb(RINGBUF *rb, uint8_t *bufOut, U16* len,
		U16 maxBufLen);
#endif

