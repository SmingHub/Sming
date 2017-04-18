#include <Wire.h>
#include <Si7021.h>

SI7021 si7021;

// TIMINGS DEMO
// ------------
// Prints information to the serial monitor showing how long each function takes to return.
// The timing is done in microseconds.

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
uint16_t delayS = 2000;
uint32_t startMicros, endMicros;

Serial.begin(115200);
si7021.begin(); // Runs : Wire.begin() + reset()
  while(!Serial);
delay(2000);

Serial.println("FUNCTION CALLED\t\t\t\t\t\tTIME (Microseconds)");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

startMicros = micros(); si7021.checkVDD(); endMicros = micros();
Serial.print("si7021.checkVDD();\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.getDeviceID(); endMicros = micros();
Serial.print("si7021.getDeviceID();\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.getFirmwareVer(); endMicros = micros();
Serial.print("si7021.getFirmwareVer();\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.getHumidityRes(); endMicros = micros();
Serial.print("si7021.getHumidityRes();\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.getTempRes(); endMicros = micros();
Serial.print("si7021.getTempRes();\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.setHeater(true); endMicros = micros(); si7021.setHeater(false);
Serial.print("si7021.setHeater(true);\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.getHeater(); endMicros = micros();
Serial.print("si7021.getHeater();\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.setHumidityRes(12); endMicros = micros();
Serial.print("si7021.setHumidityRes(12);\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.readHumidity(); endMicros = micros();
Serial.print("si7021.readHumidity();\t(12-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setHumidityRes(11);
startMicros = micros(); si7021.readHumidity(); endMicros = micros();
Serial.print("si7021.readHumidity();\t(11-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setHumidityRes(10);
startMicros = micros(); si7021.readHumidity(); endMicros = micros();
Serial.print("si7021.readHumidity();\t(10-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setHumidityRes(8);
startMicros = micros(); si7021.readHumidity(); endMicros = micros();
Serial.print("si7021.readHumidity();\t (8-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.setTempRes(14); endMicros = micros();
Serial.print("si7021.setTempRes(14);\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.readTemp(); endMicros = micros();
Serial.print("si7021.readTemp();\t(14-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setTempRes(13);
startMicros = micros(); si7021.readTemp(); endMicros = micros();
Serial.print("si7021.readTemp();\t(13-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setTempRes(12);
startMicros = micros(); si7021.readTemp(); endMicros = micros();
Serial.print("si7021.readTemp();\t(12-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
si7021.setTempRes(11);
startMicros = micros(); si7021.readTemp(); endMicros = micros();
Serial.print("si7021.readTemp();\t(11-bit)\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.readTempPrev(); endMicros = micros();
Serial.print("si7021.readTempPrev();\t\t\t\t\t= "); Serial.println(endMicros - startMicros);

float humidity = si7021.readHumidity();
float temp = si7021.readTemp();
uint16_t tempInt = si7021.floatToInt(temp);
float tempFloat = si7021.intToFloat(tempInt);
float tempCtoF = si7021.CtoF(temp);
float tempFtoC = si7021.FtoC(tempCtoF);

startMicros = micros(); si7021.floatToInt(temp); endMicros = micros();
Serial.print("si7021.floatToInt(temp);\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.intToFloat(tempInt); endMicros = micros();
Serial.print("si7021.intToFloat(tempInt);\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.CtoF(temp); endMicros = micros();
Serial.print("si7021.CtoF(temp);\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
startMicros = micros(); si7021.FtoC(tempCtoF); endMicros = micros();
Serial.print("si7021.FtoC(tempCtoF);\t\t\t\t\t= "); Serial.println(endMicros - startMicros);
Serial.println("---------------------------------------------------------------------------------------");
Serial.println("Finished.");
}

void loop()
{
}

