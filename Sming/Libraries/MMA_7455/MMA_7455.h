// MMA_7455.h - 3 Axis Accelerometer Library
// Moritz Kemper, IAD Physical Computing Lab
// moritz.kemper@zhdk.ch
// ZHdK, 03/04/2012
// Released under Creative Commons Licence

#ifndef MMA_7455_h
#define MMA_7455_h

#include "Arduino.h"
#include "Wire.h"

#define MMA_7455_ADDRESS 0x1D //I2C Adsress for the sensor
#define MMA_7455_MODE_CONTROLL 0x16 //Call the sensors Mode Control

enum MMA7455Sensitivity
{
	MMA_7455_2G_MODE = 0x05, //Set Sensitivity to 2g
	MMA_7455_4G_MODE = 0x09, //Set Sensitivity to 4g
	MMA_7455_8G_MODE = 0x01 //Set Sensitivity to 8g
};

#define X_OUT 0x06 //Register for reading the X-Axis
#define Y_OUT 0x07 //Register for reading the Y-Axis
#define Z_OUT 0x08 //Register for reading the Z-Axis

class MMA_7455
{
  public:
    MMA_7455();
    void initSensitivity(MMA7455Sensitivity sensitivity);
    void calibrateOffset(float x_axis_offset, float y_axis_offset, float z_axis_offset);
    unsigned char readAxis(char axis);
  private:
    unsigned char _buffer;
	float _x_axis_offset;
	float _y_axis_offset;
	float _z_axis_offset;
};

#endif
