#include "MirfSpiDriver.h"

#ifndef __MIRF_HARDWARE_SPI85_DRIVER
#define __MIRF_HARDWARE_SPI85_DRIVER

#include <SPI85.h>

class MirfHardwareSpi85Driver : public MirfSpiDriver {

	public: 
		uint8_t transfer(uint8_t data) {
			return SPI85.transfer(data);
		}

		void begin() {
			SPI85.begin();
			SPI85.setDataMode(SPI_MODE0);
#if 0
			/* Not (yet) supported by SPI85. */
			SPI85.setClockDivider(SPI_2XCLOCK_MASK);
#endif
		}

		void end() {
			SPI85.end();
		}
};

MirfHardwareSpi85Driver MirfHardwareSpi85;

#endif
