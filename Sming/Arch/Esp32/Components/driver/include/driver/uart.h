#pragma once

#include <soc/uart_caps.h>

#define UART_PHYSICAL_COUNT SOC_UART_NUM ///< Number of physical UARTs on the system
#define UART_COUNT SOC_UART_NUM			 ///< Number of UARTs on the system, virtual or otherwise

#include_next <driver/uart.h>
