#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#pragma GCC warning "Use of 'user_config.h' is deprecated - see https://sming.readthedocs.io/en/latest/upgrading/4.4-4.5.html"

#include <esp_systemapi.h>

// Extended string conversion for compatibility
#include <stringconversion.h>

// Network base API
#ifndef DISABLE_NETWORK
#include <lwip/init.h>
#include <lwip/debug.h>
#include <lwip/stats.h>
#include <lwip/tcp.h>
#include <lwip/udp.h>
#include <lwip/dns.h>
#endif

#endif /* __USER_CONFIG_H__ */
