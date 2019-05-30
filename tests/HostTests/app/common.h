#pragma once

#include <SmingCore.h>

#ifdef ARCH_HOST
#include <hostlib/hostapi.h>
#else
#define hostmsg debug_i
#endif

#define startTest(str)                                                                                                 \
	do {                                                                                                               \
		m_puts("\r\n");                                                                                                \
		hostmsg(">> %s", PSTR(str));                                                                                   \
	} while(0)
