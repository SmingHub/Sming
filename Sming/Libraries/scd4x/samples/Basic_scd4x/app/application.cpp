#include <SmingCore.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>

#define SCD41_SDA_PIN 5
#define SCD41_SCL_PIN 18

#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

SensirionI2cScd4x sensor;
Timer scdTimer;

void scdLoop()
{
	bool dataReady = false;
	while(!dataReady) {
		auto scd4xError = sensor.getDataReadyStatus(dataReady);
		if(scd4xError != NO_ERROR) {
			Serial << _F("Error trying to execute getDataReadyStatus(): ");
			//errorToString(error, errorMessage, sizeof errorMessage);
			Serial.println(scd4xError);
			return;
		}
		delay(100);
	}

	// If ambient pressure compenstation during measurement
	// is required, you should call the respective functions here.
	// Check out the header file for the function definition.
	uint16_t co2Concentration = 0;
	float temperature = 0;
	float relativeHumidity = 0.0;
	auto scd4xError = sensor.readMeasurement(co2Concentration, temperature, relativeHumidity);
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute readMeasurement(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
		return;
	}

	Serial.printf(_F("CO2: %4d t:   %2.1f rh:    %2.0f\r\n"), co2Concentration, temperature, relativeHumidity);
}

void init()
{
	// 115200 by default, GPIO1,GPIO3, see Serial.swap(), HardwareSerial
	Serial.begin(SERIAL_BAUD_RATE, SERIAL_8N1, SERIAL_FULL);
	Serial.systemDebugOutput(true);

	Wire.pins(SCD41_SDA_PIN, SCD41_SCL_PIN);
	Wire.begin();
	sensor.begin(Wire, SCD41_I2C_ADDR_62);

	uint64_t serialNumber = 0;
	delay(30);
	// Ensure sensor is in clean state
	auto scd4xError = sensor.wakeUp();
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute wakeUp(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
	}
	scd4xError = sensor.stopPeriodicMeasurement();
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute stopPeriodicMeasurement(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
	}
	scd4xError = sensor.reinit();
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute reinit(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
	}
	// Read out information about the sensor
	scd4xError = sensor.getSerialNumber(serialNumber);
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute getSerialNumber(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
		return;
	}
	Serial << _F("serial number: ") << serialNumber << endl;
	//
	// If temperature offset and/or sensor altitude compensation
	// is required, you should call the respective functions here.
	// Check out the header file for the function definitions.
	// Start periodic measurements (5sec interval)
	scd4xError = sensor.startPeriodicMeasurement();
	if(scd4xError != NO_ERROR) {
		Serial << _F("Error trying to execute startPeriodicMeasurement(): ");
		//errorToString(error, errorMessage, sizeof errorMessage);
		Serial.println(scd4xError);
		return;
	}
	//
	// If low-power mode is required, switch to the low power
	// measurement function instead of the standard measurement
	// function above. Check out the header file for the definition.
	// For SCD41, you can also check out the single shot measurement example.

	scdTimer.initializeMs<5000>(scdLoop).start();
}
