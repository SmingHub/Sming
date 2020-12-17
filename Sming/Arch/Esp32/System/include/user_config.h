#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

// UART config
#define SERIAL_BAUD_RATE COM_SPEED_SERIAL

#include <sdkconfig.h>

#include <esp_systemapi.h>

// Extended string conversion for compatibility
#include <stringconversion.h>

// Network base API
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include <espinc/lwip_includes.h>
#pragma GCC diagnostic pop


#endif
