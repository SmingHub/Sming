
// Include the Wire library for I2C access.
#include <Wire.h>
// Include the Love Electronics BMP180 library.
#include <BMP180.h>

// Store an instance of the BMP180 sensor.
BMP180 barometer;
// We are going to use the on board LED for an indicator.
int indicatorLed = 13; 

// Store the current sea level pressure at your location in Pascals.
float seaLevelPressure = 101325;

void setup()
{
  // We start the serial library to output our messages.
  Serial.begin(9600);
  // We start the I2C on the Arduino for communication with the BMP180 sensor.
  Wire.begin();
  // Set up the Indicator LED.
  pinMode(indicatorLed, OUTPUT);
  // We create an instance of our BMP180 sensor.
  barometer = BMP180();
  // We check to see if we can connect to the sensor.
  if(barometer.EnsureConnected())
  {
    Serial.println("Connected to BMP180."); // Output we are connected to the computer.
    digitalWrite(indicatorLed, HIGH); // Set our LED.
    
     // When we have connected, we reset the device to ensure a clean start.
    barometer.SoftReset();
    // Now we initialize the sensor and pull the calibration data.
    barometer.Initialize();
  }
  else
  { 
    Serial.println("Could not connect to BMP180.");
    digitalWrite(indicatorLed, LOW); // Set our LED.
  }
}

void loop()
{
  if(barometer.IsConnected)
  {
    // Retrive the current pressure in Pascals.
    long currentPressure = barometer.GetPressure();
    
    // Print out the Pressure.
    Serial.print("Pressure: ");
    Serial.print(currentPressure);
    Serial.print(" Pa");
    
    // Retrive the current altitude (in meters). Current Sea Level Pressure is required for this.
    float altitude = barometer.GetAltitude(seaLevelPressure);
    
    // Print out the Altitude.
    Serial.print("\tAltitude: ");
    Serial.print(altitude);
    Serial.print(" m");
    
    // Retrive the current temperature in degrees celcius.
    float currentTemperature = barometer.GetTemperature();
    
    // Print out the Temperature
    Serial.print("\tTemperature: ");
    Serial.print(currentTemperature);
    Serial.write(176);
    Serial.print("C");
    
    Serial.println(); // Start a new line.
    delay(1000); // Show new results every second.
  }
}
