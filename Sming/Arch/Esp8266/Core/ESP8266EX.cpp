/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ESP8266EX.cpp
 *
 ****/

#include "Digital.h"
#include "ESP8266EX.h"
#include "Digital.h"

const EspDigitalPin EspDigitalPins[] = {
	{0, PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0}, // FLASH
	{1, PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1}, // TXD0
	{2, PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2}, // TXD1
	{3, PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3}, // RXD0
	{4, PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4},
	{5, PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5},
	{6, 0, 0}, // SD_CLK_U
	{7, 0, 0}, // SD_DATA0_U
	{8, 0, 0}, // SD_DATA1_U
	{9, PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9},
	{10, PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10},
	{11, 0, 0},								  // SD_CMD_U
	{12, PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12}, // HSPIQ
	{13, PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13}, // HSPID
	{14, PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14}, // HSPICLK
	{15, PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15}, // HSPICS
};

void EspDigitalPin::mode(uint8_t mode) const
{
	pinMode(id, mode);
}

void EspDigitalPin::write(uint8_t val) const
{
	digitalWrite(id, val);
}

uint8_t EspDigitalPin::read() const
{
	return digitalRead(id);
}
