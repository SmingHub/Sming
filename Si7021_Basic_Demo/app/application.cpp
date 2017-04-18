#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/Si7021/Si7021.h>

#define PIN_SCL	4
#define PIN_SDA	5

SI7021 si7021 = SI7021(PIN_SCL, PIN_SDA);

// BASIC DEMO
// ----------
// Print humidity and temperature to the serial monitor while toggling the heater on and off
// periodically.

// NOTE ON RESOLUTION:
// -------------------
// Changing the resolution of the temperature will also change the humidity resolution.
// Likewise, changing the resolution of the humidity will also change temperature resolution.
// Two functions are provided to change the resolution, both functions are similar except they have
// different masks to the registers, so setTempRes(14) will change the temp resolution to 14-bit
// but humidity will also be set to 12-bit. Setting the humidity resolution to 8-bit setHumidityRes(8)
// will change the temperature resolution to 12-bit.
// Resolution Table:
//                    14-bit Temp <-> 12-bit Humidity
//                    13-bit Temp <-> 10-bit Humidity
//                    12-bit Temp <->  8-bit Humidity
//                    11-bit Temp <-> 11-bit Humidity

// NOTE ON HEATER:
// ---------------
// The HTRE bit in the user register is what turns the heater on and off. This register
// is stored on non-volatile memory to it will keep its state when power is removed. If
// the heater is enabled and power is removed before the program had chance to turn it off
// then the heater will remain enabled the next time it is powered on. The heater has to
// be explicitly turned off. In the begin() function for this library, a reset procedure
// will take place and reset the user register to default, so the heater will be turned off
// and the resolution will also be set to default, 14-bit temperature and 12-bit humidity.
// Keep this in mind if testing and swapping sensors/libraries.

Timer readTimer;

void readSensor()
{
	static uint8_t heaterOnOff = 1; // Create static variable for heater control
	static int i = 20;

	if (i>=20) {
		heaterOnOff = !heaterOnOff; // Toggle heater on/off variable
		si7021.setHeater(heaterOnOff); // Turn heater on or off
		delay(100);
		Serial.print("\nHeater Status = ");
		//Serial.println(si7021.getHeater() ? "ON" : "OFF"); // getHeater doesn't work on my device ???
		Serial.println(heaterOnOff ? "ON" : "OFF");
		i=0;
		delay(100);
	}
	i++;

	Serial.print("Humidity : ");
	Serial.print(si7021.readHumidity()); // Read humidity and print to serial monitor
	Serial.print(" %\t");

	Serial.print("\tTemp : ");
	Serial.print(si7021.readTempPrev()); // Read temperature and print to serial monitor
	Serial.print(" C\t");

	Serial.println(i); // Print count down for heater change

}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	//Serial.systemDebugOutput(true); // Allow debug output to serial
	si7021.begin(); // Runs : Wire.begin() + reset()
	Serial.println("BASIC DEMO");
	Serial.println("------------------------------------------");
	si7021.setHumidityRes(12); // Humidity = 12-bit / Temperature = 14-bit

	readTimer.initializeMs(1000, readSensor).start();
}

