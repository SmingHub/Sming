#include <Wire.h>
#include <Si7021.h>

SI7021 si7021;

// FUNCTION DEMO
// -------------
// This prints a demo of the function use to the serial monitor, showing what action to expect, the
// status/return of the function and the function called.

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
uint16_t delayS = 1000;

Serial.begin(115200);
si7021.begin(); // Runs : Wire.begin() + reset()
  while(!Serial);
delay(2000);

Serial.println("ACTION TO TAKE                STATUS             FUNCTION CALLED");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

Serial.print("Check Voltage                 <--  = "); Serial.print(si7021.checkVDD() ? "OK" : "LOW");
Serial.println("\t\t si7021.checkVDD();");
Serial.print("Get: Device ID                <--  = Si"); Serial.print(si7021.getDeviceID());
Serial.println("\t si7021.getDeviceID();");
Serial.print("Get: Firmware Version         <--  = "); Serial.print(si7021.getFirmwareVer()); Serial.print(".0");
Serial.println("\t si7021.getFirmwareVer();");
Serial.print("Get: Humidity Resolution      <--  = "); Serial.print(si7021.getHumidityRes()); Serial.print("-bit");
Serial.println("\t si7021.getHumidityRes();");
Serial.print("Get: Temperature Resolution   <--  = "); Serial.print(si7021.getTempRes()); Serial.print("-bit");
Serial.println("\t si7021.getTempRes();");
Serial.print("Get: Heater Status            <--  = "); Serial.print(si7021.getHeater() ? "ON" : "OFF");
Serial.println("\t si7021.getHeaterBit();");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

Serial.print("Set: Humidity Resolution      -->  = 10-bit"); si7021.setHumidityRes(10);
Serial.println("\t si7021.setHumidityRes(10);");
Serial.print("Get: Temperature Resolution   <--  = "); Serial.print(si7021.getTempRes()); Serial.print("-bit");
Serial.println("\t si7021.getTempRes();");
Serial.print("Get: Humidity Resolution      <--  = "); Serial.print(si7021.getHumidityRes()); Serial.print("-bit");
Serial.println("\t si7021.getHumidityRes();");
Serial.print("Set: Temperature Resolution   -->  = 12-bit"); si7021.setTempRes(12);
Serial.println("\t si7021.setTempRes(12);");
Serial.print("Get: Temperature Resolution   <--  = "); Serial.print(si7021.getTempRes()); Serial.print("-bit");
Serial.println("\t si7021.getTempRes();");
Serial.print("Get: Humidity Resolution      <--  = "); Serial.print(si7021.getHumidityRes()); Serial.print("-bit");
Serial.println("\t si7021.getHumidityRes();");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

Serial.print("Set: Heater Status            -->  = ON"); si7021.setHeater(true);
Serial.println("\t\t si7021.setHeater(true);");
Serial.print("Get: Heater Status            <--  = "); Serial.print(si7021.getHeater() ? "ON" : "OFF");
Serial.println("\t\t si7021.getHeater();");
Serial.print("Set: Heater Status            -->  = OFF"); si7021.setHeater(false);
Serial.println("\t si7021.setHeater(false);");
Serial.print("Get: Heater Status            <--  = "); Serial.print(si7021.getHeater() ? "ON" : "OFF");
Serial.println("\t si7021.getHeater();");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

/* This section sets the temp resolution to 11-bit, a reset will set it back to 14-bit (default)*/
si7021.setTempRes(11);  // Sets the user register resolution bits so a reset will clear them
uint8_t tempRes1 = si7021.getTempRes(); // Read the resolution bits before reset (11-bit)
si7021.reset(); // Reset the sensors registers to default (this wil set temp resolution to 14-bit)
uint8_t tempRes2 = si7021.getTempRes(); // Read the new resolution bits from the register (14-bit)
uint8_t result = 0; // Assume FAIL and let the IF statement change to OK if success
  if(tempRes1 == 11 && tempRes2 == 14) result = 1;
/* This section sets the temp resolution to 11-bit, a reset will set it back to 14-bit (default)*/

Serial.print("Reset Sensor Registers        -->  = "); Serial.print(result ? "OK" : "FAIL");
Serial.println("\t\t si7021.reset();");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

Serial.print("Read Humidity                 <--  = "); Serial.print(si7021.readHumidity());
Serial.print(" %"); Serial.println("\t si7021.readHumidity();");
Serial.print("Read Temperature              <--  = "); Serial.print(si7021.readTemp());
Serial.print(" C"); Serial.println("\t si7021.readTemp();");
Serial.print("Read Temperature Previous     <--  = "); Serial.print(si7021.readTempPrev());
Serial.print(" C"); Serial.println("\t si7021.readTempPrev();");
Serial.println("---------------------------------------------------------------------------------------");
delay(delayS);

float humidity = si7021.readHumidity();
float temp = si7021.readTempPrev();
uint16_t tempInt = si7021.floatToInt(temp);
float tempFloat = si7021.intToFloat(tempInt);
float tempCtoF = si7021.CtoF(temp);
float tempFtoC = si7021.FtoC(tempCtoF);

Serial.print("Convert Float To Int          <--  = "); Serial.print(tempInt);
Serial.print(" C"); Serial.println("\t si7021.floatToInt(temp);");
Serial.print("Convert Int To Float          <--  = "); Serial.print(tempFloat);
Serial.print(" C"); Serial.println("\t si7021.intToFloat(tempInt);");
Serial.print("Convert Celsius To Fahrenheit <--  = "); Serial.print(tempCtoF);
Serial.print(" F"); Serial.println("\t si7021.CtoF(temp);");
Serial.print("Convert Fahrenheit To Celsius <--  = "); Serial.print(tempFtoC);
Serial.print(" C"); Serial.println("\t si7021.FtoC(tempCtoF);");
Serial.println("---------------------------------------------------------------------------------------");
Serial.println("Finished.");
}

void loop()
{
}

