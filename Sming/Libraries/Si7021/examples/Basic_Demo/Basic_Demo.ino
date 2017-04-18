#include <Wire.h>
#include <Si7021.h>

SI7021 si7021;

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

void setup()
{
Serial.begin(115200);
si7021.begin(); // Runs : Wire.begin() + reset()
  while(!Serial); // Wait for serial monitor to open
Serial.println("BASIC DEMO");
Serial.println("------------------------------------------");
si7021.setHumidityRes(12); // Humidity = 12-bit / Temperature = 14-bit
}

void loop()
{
static uint8_t heaterOnOff; // Create static variable for heater control
si7021.setHeater(heaterOnOff); // Turn heater on or off
Serial.print("Heater Status = ");
Serial.println(si7021.getHeater() ? "ON" : "OFF");

  for(int i = (heaterOnOff ? 20 : 30); i>0; i--)
  {
  Serial.print("Humidity : ");
  Serial.print(si7021.readHumidity()); // Read humidity and print to serial monitor
  Serial.print(" %\t");
  Serial.print("Temp : ");
  Serial.print(si7021.readTemp()); // Read temperature and print to serial monitor
  Serial.print(" C\t");
  Serial.println(i); // Print count down for heater change
  delay(500);
  }

heaterOnOff = !heaterOnOff; // Toggle heater on/off variable
}

