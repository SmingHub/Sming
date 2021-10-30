/*
 * twi_arch.h - Platform-specific code
 *
 * See Sming/Core/si2c.cpp
 *
 */

#pragma once

//Enable SDA (becomes output and since GPO is 0 for the pin, it will pull the line low)
#define SDA_LOW()
//Disable SDA (becomes input and since it has pullup it will go high)
#define SDA_HIGH()
#define SDA_READ() 0
#define SCL_LOW()
#define SCL_HIGH()
#define SCL_READ() 1

#define DEFAULT_SDA_PIN 4
#define DEFAULT_SCL_PIN 5
