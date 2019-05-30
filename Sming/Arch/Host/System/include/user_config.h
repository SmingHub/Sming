#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// UART config
#define SERIAL_BAUD_RATE COM_SPEED_SERIAL

#include <esp_systemapi.h>

// Extended string conversion for compatibility
#include <stringconversion.h>

// Network base API
#include <lwip/init.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/dns.h>

#endif /* __USER_CONFIG_H__ */
