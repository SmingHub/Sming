/*
 * twi_arch.h - Platform-specific code
 *
 * See Sming/Core/si2c.cpp
 *
 */

#pragma once

//Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_LOW() (GPES = (1 << twi_sda))
//Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_HIGH() (GPEC = (1 << twi_sda))
#define SDA_READ() ((GPI & (1 << twi_sda)) != 0)
#define SCL_LOW() (GPES = (1 << twi_scl))
#define SCL_HIGH() (GPEC = (1 << twi_scl))
#define SCL_READ() ((GPI & (1 << twi_scl)) != 0)
