// --------------------------------------
// i2c_scanner (Sming version)
//
// Version 1
//    This program (or code that looks like it)
//    can be found in many places.
//    For example on the Arduino.cc forum.
//    The original author is not know.
// Version 2, June 2012, Using Arduino 1.0.1
//     Adapted to be as simple as possible by Arduino.cc user Krodal
// Version 3, Feb 26  2013
//    V3 by louarnold
// Version 4, March 3, 2013, Using Arduino 1.0.3
//    by Arduino.cc user Krodal.
//    Changes by louarnold removed.
//    Scanning addresses changed from 0...127 to 1...119,
//    according to the i2c scanner by Nick Gammon
//    http://www.gammon.com.au/forum/?id=10896
// Version 5, March 28, 2013
//    As version 4, but address scans now to 127.
//    A sensor seems to use address 120.
// Version 6, March 15  2015
//    Sming version by Skurydin Alexey
//
//
// This sketch tests the standard 7-bit addresses
// Devices with higher bit address might not be seen properly.
//

#include <SmingCore.h>

namespace
{
SimpleTimer procTimer;
}

void scanBus()
{
	Serial.println("Scanning...");

	unsigned nDevices{0};
	for(uint8_t address = 1; address < 127; address++) {
		// The i2c_scanner uses the return value of
		// the Write.endTransmisstion to see if
		// a device did acknowledge to the address.
		Wire.beginTransmission(address);
		auto error = Wire.endTransmission();

		WDT.alive(); // Second option: notify Watch Dog what you are alive (feed it)

		if(error == 0) {
			Serial << _F("I2C device found at address 0x") << String(address, HEX, 2) << "  !" << endl;
			nDevices++;
		} else if(error == 4) {
			Serial << _F("Unknown error at address 0x") << String(address, HEX, 2) << endl;
		}
	}

	Serial << nDevices << _F(" I2C devices found.") << endl << endl;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output

	WDT.enable(false); // First (but not the best) option: fully disable watch dog timer

	// Default I2C pins (SDA: 2, SCL:0)

	// You can change pins:
	//Wire.pins(14, 12); // SDA, SCL

	Wire.begin();
	procTimer.initializeMs<3000>(scanBus).start();
}
