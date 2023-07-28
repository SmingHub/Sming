# MPU6050 Six-Axis (Gyro + Accelerometer)

Based on code from [jrowberg/i2cdevlib](https://github.com/jrowberg/i2cdevlib/tree/master/ESP32_ESP-IDF/components/MPU6050). Most of the code is the same, except:

- Removed MPU6050::ReadRegister function due to incompatibility. It is also not used anywhere in the original code.
- Removed map function in favor of the Sming built-in one.
- Adapted include path and applied clangformat


