#ifndef __MIRF_SPI_DRIVER
#define __MIRF_SPI_DRIVER

class MirfSpiDriver {
	public:
		virtual uint8_t transfer(uint8_t data);

		virtual void begin();
		virtual void end();
};

#endif
