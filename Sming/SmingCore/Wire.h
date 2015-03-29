/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_WIRE_H_
#define _SMING_CORE_WIRE_H_

#include "../Wiring/WiringFrameworkDependencies.h"
#include "../Wiring/Stream.h"

// Default values
#define I2C_DEFAULT_SCL_PIN		0
#define I2C_DEFAULT_SDA_PIN		2
#define BUFFER_LENGTH			48

class SoftI2cMaster;

class TwoWire : public Stream
{
public:
	TwoWire(int pinSCL, int pinSDA);
	virtual ~TwoWire();

	void pins(int pinSCL, int pinSDA); // Can be called only before begin()
	void begin();

	void beginTransmission(uint8_t address);
	uint8_t endTransmission(bool sendStop = true);
	uint8_t requestFrom(int address, int quantity, bool sendStop = true);

	virtual int available();
	virtual int read();
	virtual int peek();
	virtual void flush();
	size_t write(uint8_t data);
	size_t write(const uint8_t *data, size_t quantity);

protected:
	uint8_t pushData();

private:
	SoftI2cMaster* master;
	int SCL;
	int SDA;
	int targetAddress;

	uint8_t txBuf[BUFFER_LENGTH];
	int txLen;

	uint8_t rxBuf[BUFFER_LENGTH];
	int rxLen;
	int rxPos;
};

extern TwoWire Wire;

#endif /* _SMING_CORE_WIRE_H_ */
