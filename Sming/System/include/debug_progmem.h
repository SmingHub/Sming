/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * debug_progmem.h
 *
 *  Contains debug functions that facilitate using strings stored in flash(irom). 
 *  This frees up RAM of all const char* debug strings 
 *
 *  Created on: 27.01.2017
 *  Author: (github.com/)ADiea
 *
 ****/

#pragma once

#include "FakePgmSpace.h"

#ifdef __cplusplus
extern "C" {
#endif

//This enables or disables logging
//Can be overridden in Makefile
#ifndef DEBUG_BUILD
	#ifdef SMING_RELEASE
		#define DEBUG_BUILD 0
	#else
		#define DEBUG_BUILD 1
	#endif
#endif

//This enables or disables file and number printing for each log line
//Can be overridden in Makefile
#ifndef DEBUG_PRINT_FILENAME_AND_LINE
#define DEBUG_PRINT_FILENAME_AND_LINE 0
#endif

#define ERR 0	
#define WARN 1
#define INFO 2
#define DBG 3

//This sets debug verbose level
//Define in Makefile, default is INFO
#ifndef DEBUG_VERBOSE_LEVEL
#define DEBUG_VERBOSE_LEVEL INFO
#endif

// Dummy to omit debug information safely
#define debug_none(fmt, ...)                                                                                           \
	do {                                                                                                               \
	} while(0)

#if DEBUG_BUILD

// http://stackoverflow.com/a/35441900
#define MACROCAT2(x,y,z) x##y##z
#define MACROCONCAT(x,y,z) MACROCAT2(x,y,z)
#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

#ifdef ICACHE_FLASH
#define PROGMEM_DEBUG                                                                                                  \
	__attribute__((aligned(4))) __attribute__((section(MACROQUOTE(MACROCONCAT(.irom.debug., __COUNTER__, __LINE__)))))
#else
#define PROGMEM_DEBUG
#endif

extern uint32_t system_get_time();

//A static const char[] is defined having a unique name (log_ prefix, filename and line number)
//This will be stored in the irom section(on flash) freeing up the RAM
//Next special version of printf from FakePgmSpace is called to fetch and print the message
#if DEBUG_PRINT_FILENAME_AND_LINE
#define debug_e(fmt, ...)                                                                                              \
	(__extension__({                                                                                                   \
		static const char log_string[] PROGMEM_DEBUG = "[" MACROQUOTE(CUST_FILE_BASE) ":%d] " fmt "\r\n";                \
		LOAD_PSTR(fmtbuf, log_string);                                                                                 \
		m_printf(fmtbuf, __LINE__, ##__VA_ARGS__);                                                                     \
	}))
#else
#define debug_e(fmt, ...)                                                                                              \
	(__extension__({                                                                                                   \
		static const char log_string[] PROGMEM_DEBUG = "%u " fmt "\r\n";                                                 \
		LOAD_PSTR(fmtbuf, log_string);                                                                                 \
		m_printf(fmtbuf, system_get_time(), ##__VA_ARGS__);                                                            \
	}))
#endif

/*
 * Print a block of data but only at or above given debug level
 */
#define debug_hex(_level, _tag, _data, _len, ...)                                                                      \
	{                                                                                                                  \
		if(DEBUG_VERBOSE_LEVEL >= _level)                                                                              \
			m_printHex(_F(_tag), _data, _len, ##__VA_ARGS__);                                                          \
	}

	#if DEBUG_VERBOSE_LEVEL == DBG
		#define debug_w debug_e
		#define debug_i debug_e
		#define debug_d debug_e
	#elif DEBUG_VERBOSE_LEVEL == INFO
		#define debug_w debug_e
		#define debug_i debug_e
		#define debug_d debug_none
	#elif DEBUG_VERBOSE_LEVEL == WARN
		#define debug_w debug_e
		#define debug_i debug_none
		#define debug_d debug_none
	#else
		#define debug_w debug_none
		#define debug_i debug_none
		#define debug_d debug_none
	#endif

#else /*DEBUG_BUILD*/

#define debug_e debug_none
#define debug_w debug_none
#define debug_i debug_none
#define debug_d debug_none

#define debug_hex(_level, _tag, _data, _len, ...)                                                                      \
	do {                                                                                                               \
	} while(0)

#endif /*DEBUG_BUILD*/

#ifdef __cplusplus
}
#endif
