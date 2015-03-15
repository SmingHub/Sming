 /*
  This is a simple code to test BH1750FVI Light senosr
  communicate using I2C Protocol 
  this library enable 2 slave device address
  Main address  0x23 
  secondary address 0x5C 
  connect this sensor as following :
  VCC >>> 3.3V
  SDA >>> A4 
  SCL >>> A5
  addr >> A3
  Gnd >>>Gnd

  Written By : Mohannad Rawashdeh
  
 */
 
 // First define the library :
 #include <BH1750FVI.h> // Sensor Library
 #include <Wire.h> // I2C Library
 #include <LiquidCrystal.h>

 LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
 uint16_t Light_Intensity=0;
 // Call the function 

 BH1750FVI LightSensor;
 

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  lcd.begin(16, 2);
 
 //  call begin Function so turn the sensor On .
  LightSensor.begin();
  LightSensor.SetAddress(Device_Address_H); //Address 0x5C
  LightSensor.SetMode(Continuous_H_resolution_Mode);
  lcd.setCursor(0, 0);
  lcd.print("BH1750 Sensor");
  lcd.setCursor(1, 1);
  lcd.print("Please wait..."); 
  delay(3000);
  lcd.clear();
 
}
 
 void loop() {
  // put your main code here, to run repeatedly: 
   lcd.clear();
   lcd.setCursor(0, 0);
   lcd.print(" Intensity = ");
   lcd.setCursor(5, 1);
   Light_Intensity = LightSensor.GetLightIntensity();
   lcd.print(Light_Intensity);
   lcd.print(" Lux");
   delay(2000);
   
  
}
