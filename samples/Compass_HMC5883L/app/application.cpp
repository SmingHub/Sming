#include <SmingCore.h>
#include <Libraries/HMC5883L/HMC5883L.h>

// class default I2C address is 0x1E
// specific I2C addresses may be passed as a parameter here
// this device only supports one I2C address (0x1E)
HMC5883L mag;
int16_t mx, my, mz;

Timer procTimer;

void readCompass();

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);  // 115200 by default
	Serial.systemDebugOutput(false); // Disable debug output to serial

	// join I2C bus (I2Cdev library doesn't do this automatically)
	Wire.begin();

	mag.initialize();

	if(mag.testConnection()) {
		Serial.println(_F("[Compass] Magnetometer found"));
	} else {
		Serial.println(_F("Can't connect to Magnetometer"));
	}

	// Start reading loop
	procTimer.initializeMs(100, readCompass).start();
}

void readCompass()
{
	// read raw heading measurements from device
	mag.getHeading(&mx, &my, &mz);

	// To calculate heading in degrees. 0 degree indicates North
	float heading = atan2(my, mx);
	if(heading < 0) {
		heading += 2 * PI;
	} else if(heading > 2 * PI) {
		heading -= 2 * PI;
	}

	// display tab-separated gyro x/y/z values and heading
	Serial << "mag:\t" << mx << '\t' << my << '\t' << mz << "\theading:\t" << heading * RAD_TO_DEG << endl;
}
