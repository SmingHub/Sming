#include <Libraries/MPU6050/MPU6050.h>
#include <SmingCore.h>

constexpr float main_loop_interval = 0.02; // sec
SimpleTimer main_loop_timer;
MPU6050 mpu;

void print_accel_gyro(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz)
{
	Serial.print("a/g:\t");
	Serial.print(ax);
	Serial.print("\t");
	Serial.print(ay);
	Serial.print("\t");
	Serial.print(az);
	Serial.print("\t");
	Serial.print(gx);
	Serial.print("\t");
	Serial.print(gy);
	Serial.print("\t");
	Serial.println(gz);
}

void main_loop()
{
	int16_t ax, ay, az;
	int16_t gx, gy, gz;
	mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
	print_accel_gyro(ax, ay, az, gx, gy, gz);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	Wire.begin(DEFAULT_SDA_PIN, DEFAULT_SCL_PIN);
	mpu.initialize();
	Serial.println(mpu.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

	main_loop_timer.initializeMs(static_cast<int>(main_loop_interval * 1000), main_loop).start();
}
