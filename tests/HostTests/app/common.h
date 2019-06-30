#pragma once

#include <SmingCore.h>

#define startTest(str)                                                                                                 \
	do {                                                                                                               \
		m_puts("\r\n");                                                                                                \
		m_printf(_F(">> %s\r\n"), PSTR(str));                                                                          \
	} while(0)
