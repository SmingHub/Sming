#include <Libraries/MPU6050/MPU6050.h>
#include <SmingCore.h>

constexpr uint16_t mainLoopInterval = 20; // ms
SimpleTimer mainLoopTimer;
MPU6050 mpu;

void mainLoop()
{
	const MPU6050::Motion6 accelGyro = mpu.getMotion6();
	Serial << accelGyro << endl;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	Wire.begin(DEFAULT_SDA_PIN, DEFAULT_SCL_PIN);
	mpu.initialize();
	bool success = mpu.testConnection();
	Serial << _F("MPU6050 connection ") << (success ? _F("successful") : _F("failed")) << endl;

	mainLoopTimer.initializeMs<mainLoopInterval>(mainLoop).start();
}
