/*
 * twi_arch.h - Platform-specific code
 *
 * See Sming/Core/si2c.cpp
 *
 */

#pragma once

#include <hal/gpio_types.h>
#define gpio_drive_cap_t uint32_t
#include <hal/gpio_ll.h>

//Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_LOW() gpio_ll_output_enable(&GPIO, gpio_num_t(twi_sda))
//Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_HIGH() gpio_ll_output_disable(&GPIO, gpio_num_t(twi_sda))
#define SDA_READ() (gpio_ll_get_level(&GPIO, gpio_num_t(twi_sda)) != 0)
#define SCL_LOW() gpio_ll_output_enable(&GPIO, gpio_num_t(twi_scl))
#define SCL_HIGH() gpio_ll_output_disable(&GPIO, gpio_num_t(twi_scl))
#define SCL_READ() (gpio_ll_get_level(&GPIO, gpio_num_t(twi_scl)) != 0)

#define DEFAULT_SDA_PIN 21
#define DEFAULT_SCL_PIN 22
