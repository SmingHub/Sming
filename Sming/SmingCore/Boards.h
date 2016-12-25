/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_BOARDS_H_
#define _SMING_CORE_BOARDS_H_

#include "../SmingCore/ESP8266EX.h"

// BETA state. Will be rewritten with new interface.
///@todo Is ESP01_Description required? Source says it will be removed with new interface

#ifdef BOARD_ESP01

class ESP01_Description
{
public:
	EspDigitalPin gpio0 = EspDigitalPins[0];
	EspDigitalPin gpio2 = EspDigitalPins[2];
	EspDigitalPin tx = EspDigitalPins[1];
	EspDigitalPin rx = EspDigitalPins[3];
};

static ESP01_Description ESP01;

#endif

#endif /* _SMING_CORE_BOARDS_H_ */
