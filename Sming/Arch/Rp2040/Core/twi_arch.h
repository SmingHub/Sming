/*
 * twi_arch.h - Platform-specific code
 *
 * See Sming/Core/si2c.cpp
 *
 */

#pragma once

#include <hardware/gpio.h>
#include <pico.h>

//Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_LOW() gpio_set_dir(twi_sda, true)
//Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_HIGH() gpio_set_dir(twi_sda, false)
#define SDA_READ() gpio_get(twi_sda)
#define SCL_LOW() gpio_set_dir(twi_scl, true)
#define SCL_HIGH() gpio_set_dir(twi_scl, false)
#define SCL_READ() gpio_get(twi_scl)

#define DEFAULT_SDA_PIN PICO_DEFAULT_I2C_SDA_PIN
#define DEFAULT_SCL_PIN PICO_DEFAULT_I2C_SCL_PIN
