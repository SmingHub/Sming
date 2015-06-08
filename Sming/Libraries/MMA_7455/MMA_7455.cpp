// MMA_7455.cpp - 3 Axis Accelerometer Library
// Moritz Kemper, IAD Physical Computing Lab
// moritz.kemper@zhdk.ch
// ZHdK, 03/04/2012
// Released under Creative Commons Licence

#include "MMA_7455.h"

MMA_7455::MMA_7455()
{
}

void MMA_7455::initSensitivity(MMA7455Sensitivity sensitivity)
{
  Wire.begin();
  delay(1000);
  Wire.beginTransmission(MMA_7455_ADDRESS);
  Wire.write(MMA_7455_MODE_CONTROLL);
  Wire.write(sensitivity);
  Wire.endTransmission();
  delay(1000);
}

void MMA_7455::calibrateOffset(float x_axis_offset, float y_axis_offset, float z_axis_offset)
{
  _x_axis_offset = x_axis_offset;
  _y_axis_offset = y_axis_offset;
  _z_axis_offset = z_axis_offset;
}
unsigned char MMA_7455::readAxis(char axis)
{
  Wire.beginTransmission(MMA_7455_ADDRESS);
  if(axis == 'x' || axis == 'X')
  {
    Wire.write(X_OUT);
  }
  if(axis == 'y' || axis == 'Y')
  {
    Wire.write(Y_OUT);
  }
  if(axis == 'z' || axis == 'Z')
  {
    Wire.write(Z_OUT);
  }
  Wire.endTransmission();
  Wire.beginTransmission(MMA_7455_ADDRESS);
  Wire.requestFrom(MMA_7455_ADDRESS, 1);
  if(Wire.available())
  {
    _buffer = Wire.read();
  }
  else
	  debugf("No axis data");
  if(axis == 'x' || axis == 'X')
  {
    _buffer = _buffer + _x_axis_offset;
  }
  if(axis == 'y' || axis == 'Y')
  {
    _buffer = _buffer + _y_axis_offset;
  }
  if(axis == 'z' || axis == 'Z')
  {
    _buffer = _buffer + _z_axis_offset;
  }

  return _buffer;
}
