/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * fast_io.h
 *
 * @author mikee47 January 2022
 *
 * Support macros to perform GPIO as fast as possible by avoiding conditionals and accessing memory-mapped registers directly.
 *
 ****/

#pragma once

#include <sming_attr.h>

/**
 * @brief Return value if flag is clear, otherwise 0
 * @param flag Only bit 0 is checked
 * @param value if flag is clear
 */
#define GP_IF0(flag, value) ((value) & (((flag)&1) - 1))

/**
 * @brief Return value if flag is set, otherwise 0
 * @param flag Only bit 0 is checked
 * @param value if flag is set
 */
#define GP_IF1(flag, value) ((value) & ~(((flag)&1) - 1))

/**
 * @brief Choose one of two values depending on whether \a flag is 0 or 1
 * @param flag Only bit 0 is checked
 * @param value0 if flag is clear
 * @param value1 if flag is set
 */
#define GP_SELECT(flag, value0, value1) (((value0) & (((flag)&1) - 1)) | ((value1) & ~(((flag)&1) - 1)))

/**
 * @brief Read a GPIO into bit 0
 * @param pin Pin number to read
 * @param reg GPIO address for INPUT
 * @retval uint32_t Either 0 or 1
 */
#define GP_FAST_READ(pin, reg) ((*(const volatile uint32_t*)(reg) >> ((pin)&31)) & 1)

/**
 * @brief Set or clear a GPIO line
 * @param pin Number of GPIO pin to write
 * @param regclr Absolute register address used to CLEAR a GPIO to 0
 * @param regset Absolute register address used to SET a GPIO to 1
 */
#define GP_FAST_WRITE(pin, val, regclr, regset) (*(volatile uint32_t*)GP_SELECT(val, regclr, regset) = BIT(pin))
