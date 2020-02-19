#include <SmingCore.h>
#include <Adafruit_VL53L0X.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();

// GPIO - NodeMCU pins
#define SDA 4	// D2
#define SCL 5	// D1
#define XSHUT 14 // D5
#define INT 12   // D6

Timer loopTimer;

void loop() {
  VL53L0X_RangingMeasurementData_t measure;

  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    Serial.print("Distance (mm): "); Serial.println(measure.RangeMilliMeter);
  } else {
    Serial.println(" out of range ");
  }
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// WIFI not needed for demo. So disabling WIFI.
	WifiStation.enable(false);
	WifiAccessPoint.enable(false);

	// Create components.
	Wire.begin(SDA, SCL);

	lox.setLongRangeMode(true);

	if (!lox.begin()) {
		Serial.println(F("Failed to boot VL53L0X"));
		while (1) {}
	}

	Serial.println(F("VL53L0X API Simple Ranging example\n\n"));
	loopTimer.initializeMs(100, loop).start();
}
