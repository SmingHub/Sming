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
	Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

	mainLoopTimer.initializeMs<mainLoopInterval>(mainLoop).start();
}
