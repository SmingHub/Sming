#include <SmingCore.h>
#include <Libraries/BH1750FVI/BH1750FVI.h>

/*
Set the Address pin state to change I2C address:
BH1750FVI_ADDRESS_LOW	"0x23" - usually by default
BH1750FVI_ADDRESS_HIGH	"0x5C"
*/
BH1750FVI LightSensor(BH1750FVI_ADDRESS_LOW);

Timer procTimer;

void readLight()
{
	Serial.println("Reading");
	uint16_t lux = LightSensor.getLightIntensity(); // Get Lux value
	Serial.print("Light: ");
	Serial.print(lux);
	Serial.println(" lux");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output to serial

	if(LightSensor.begin() == 0)
		Serial.println("LightSensor initialized");
	else
		Serial.println("LightSensor not available. May be wrong I2C address?");

	/*
	Set the Working Mode for this sensor
	The data sheet recommended To use Continuous_H_resolution_Mode
	*/
	LightSensor.setMode(BH1750_Continuous_H_resolution_Mode);

	// Start reading loop
	procTimer.initializeMs(300, readLight).start();
}
