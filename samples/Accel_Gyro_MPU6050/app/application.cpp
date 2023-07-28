#include <Libraries/MPU6050/MPU6050.h>
#include <SmingCore.h>

constexpr uint16_t mainLoopInterval = 20; // ms
SimpleTimer mainLoopTimer;
MPU6050 mpu;

void printAccelGyro(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz)
{
	Serial << "a/g:\t" << ax << "\t" << ay << "\t" << az << "\t" << gx << "\t" << gy << "\t" << gz << endl;
}

void mainLoop()
{
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	printAccelGyro(ax, ay, az, gx, gy, gz);
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
