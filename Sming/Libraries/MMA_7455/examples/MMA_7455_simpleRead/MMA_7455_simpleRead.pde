// Example which uses the MMA_7455 library
// Moritz Kemper, IAD Physical Computing Lab
// moritz.kemper@zhdk.ch
// ZHdK, 03/04/2012
// Released under Creative Commons Licence

#include <Wire.h> //Include the Wire library
#include <MMA_7455.h> //Include the MMA_7455 library

MMA_7455 mySensor = MMA_7455(); //Make an instance of MMA_7455

char xVal, yVal, zVal; //Variables for the values from the sensor

void setup()
{
  Serial.begin(9600);
  // Set the sensitivity you want to use
  // 2 = 2g, 4 = 4g, 8 = 8g
  mySensor.initSensitivity(2);
  // Calibrate the Offset, that values corespond in 
  // flat position to: xVal = -30, yVal = -20, zVal = +20
  // !!!Activate this after having the first values read out!!!
  //mySensor.calibrateOffset(0.23, -43.2, 12.43);
}

void loop()
{
  xVal = mySensor.readAxis('x'); //Read out the 'x' Axis
  yVal = mySensor.readAxis('y'); //Read out the 'y' Axis
  zVal = mySensor.readAxis('z'); //Read out the 'z' Axis
}
