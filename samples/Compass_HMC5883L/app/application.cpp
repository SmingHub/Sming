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

	if(mag.testConnection())
		Serial.println("[Compass] Magnetometer found");
	else
		Serial.println("Can't connect to Magnetometer");

	// Start reading loop
	procTimer.initializeMs(100, readCompass).start();
}

void readCompass()
{
	// read raw heading measurements from device
	mag.getHeading(&mx, &my, &mz);

	// display tab-separated gyro x/y/z values
	Serial.print("mag:\t");
	Serial.print(mx);
	Serial.print("\t");
	Serial.print(my);
	Serial.print("\t");
	Serial.print(mz);
	Serial.print("\t");

	// To calculate heading in degrees. 0 degree indicates North
	float heading = atan2(my, mx);
	if(heading < 0)
		heading += 2 * PI;
	if(heading > 2 * PI)
		heading -= 2 * PI;
	Serial.print("heading:\t");
	Serial.println(heading * RAD_TO_DEG);
}
