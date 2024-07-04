#pragma once

#include <esp_idf_version.h>
#include <soc/soc_caps.h>

#if SOC_USB_SERIAL_JTAG_SUPPORTED && ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 2, 0)
#define UART_PHYSICAL_COUNT (SOC_UART_NUM + 1) ///< Number of physical UARTs on the system
#define UART_ID_SERIAL_USB_JTAG SOC_UART_NUM
#else
#define UART_PHYSICAL_COUNT SOC_UART_NUM ///< Number of physical UARTs on the system
#endif

#define UART_COUNT SOC_UART_NUM ///< Number of UARTs on the system, virtual or otherwise

#include_next <driver/uart.h>
