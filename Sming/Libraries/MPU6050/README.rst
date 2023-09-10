API Documentation
-----------------

.. doxygenclass:: MPU6050
   :members:

# MPU6050 Six-Axis (Gyro + Accelerometer)
Based on code from [jrowberg/i2cdevlib](https://github.com/jrowberg/i2cdevlib/tree/master/ESP32_ESP-IDF/components/MPU6050) @ 605a740. Most of the code is the same, except:

-   Removed MPU6050::ReadRegister function due to incompatibility. It is also not used anywhere in the original code.
-   MPU6050_6Axis_MotionApps20.h and MPU6050_9Axis_MotionApps41.h are not included due to deps to freeRTOS. helper_3dmath.h is also not included since it is only used in the above mentioned files.
-   Removed map function in favor of the Sming built-in one.
-   Adapted include path, coding style and applied clangformat
