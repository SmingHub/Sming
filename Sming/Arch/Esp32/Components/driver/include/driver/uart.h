#pragma once

#include <soc/uart_caps.h>

#define UART0 0
#define UART1 1
#define UART2 2							 ///< Virtualised UART0
#define UART_PHYSICAL_COUNT UART_NUM_MAX ///< Number of physical UARTs on the system
#define UART_COUNT UART_NUM_MAX			 ///< Number of UARTs on the system, virtual or otherwise

#include_next <driver/uart.h>
