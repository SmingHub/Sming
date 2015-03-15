/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_SPI_H_
#define _SMING_CORE_SPI_H_

#include "../Wiring/WiringFrameworkDependencies.h"

#define SPI_ID_MAIN         0
#define SPI_ID_HSPI         1

class SPIClass {
public:
	SPIClass(uint8_t spiID);

	void begin(); // Default
	void end();

	void transfer(uint8_t * data, uint8_t count);
	byte transfer(uint8_t data);
	//void setBitOrder(uint8_t bitOrder); // MSBFIRST or LSBFIRST

protected:
	void writeData(uint8_t * data, uint8_t numberByte);
	void readData(uint8_t * data, uint8_t numberByte);

private:
	uint8_t id;
	//uint8_t order; // = MSBFIRST
};

extern SPIClass SPI;

#endif /* _SMING_CORE_SPI_H_ */
