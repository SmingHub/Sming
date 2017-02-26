/*
 * debug_progmem.h
 *
 *  Contains debug functions that facilitate using strings stored in flash(irom). 
 *  This frees up RAM of all const char* debug strings 
 *
 *  Created on: 27.01.2017
 *  Author: (github.com/)ADiea
 */
#ifndef DEBUG_PROGMEM_H
#define DEBUG_PROGMEM_H

#include <stdarg.h>
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

#if DEBUG_BUILD

// http://stackoverflow.com/a/35441900
#define MACROCAT2(x,y,z) x##y##z
#define MACROCONCAT(x,y,z) MACROCAT2(x,y,z)
#define MACROQUOT(x) #x
#define MACROQUOTE(x) MACROQUOT(x)

//A static const char[] is defined having a unique name (log_ prefix, filename and line number)
//This will be stored in the irom section(on flash) feeing up the RAM
//Next special version of printf from FakePgmSpace is called to fetch and print the message
#if DEBUG_PRINT_FILENAME_AND_LINE
#define debug_e(fmt, ...) \
	({static const char log_string[] \
	__attribute__((aligned(4))) \
	__attribute__((section(MACROQUOTE(MACROCONCAT(.irom.debug.,__COUNTER__,__LINE__))))) = "[" MACROQUOTE(CUST_FILE_BASE) ":%d] " fmt "\n"; \
	printf_P_stack(log_string, __LINE__, ##__VA_ARGS__);})
#else
#define debug_e(fmt, ...) \
	({static const char log_string[] \
	__attribute__((aligned(4))) \
	__attribute__((section(MACROQUOTE(MACROCONCAT(.irom.debug.,__COUNTER__,__LINE__))))) = fmt "\n"; \
	printf_P_stack(log_string, ##__VA_ARGS__);})
#endif

	#if DEBUG_VERBOSE_LEVEL == DBG
		#define debug_w debug_e
		#define debug_i debug_e
		#define debug_d debug_e
	#elif DEBUG_VERBOSE_LEVEL == INFO
		#define debug_w debug_e
		#define debug_i debug_e
		#define debug_d
	#elif DEBUG_VERBOSE_LEVEL == WARN
		#define debug_w debug_e
		#define debug_i
		#define debug_d
	#else
		#define debug_w
		#define debug_i
		#define debug_d
	#endif
#else /*DEBUG_BUILD*/
	#define debug_e
	#define debug_w
	#define debug_i
	#define debug_d
#endif /*DEBUG_BUILD*/

#ifdef __cplusplus
}
#endif

#endif /*#ifndef DEBUG_PROGMEM_H*/
