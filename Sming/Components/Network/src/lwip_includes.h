/*
 * lwip_includes.h
 *
 *  Created on: 23 ����. 2015 �.
 *      Author: Anakonda
 */

#pragma once

#ifdef ARCH_ESP8266

// ESP SDK config
#define LWIP_OPEN_SRC

#ifdef ENABLE_ESPCONN
#include <espconn.h>
#endif

#endif

#ifdef ARCH_ESP32
#include <sdkconfig.h>
#endif

#include <lwipopts.h>
#include <lwip/init.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <lwip/ip_addr.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/dns.h>
