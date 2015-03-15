/*
 * Displays text sent over the serial port (e.g. from the Serial Monitor) on
 * an attached LCD.
 */
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define BACKLIGHT_PIN     13

LiquidCrystal_I2C lcd(0x38);  // set the LCD address to 0x38

void setup()
{
  pinMode ( BACKLIGHT_PIN, OUTPUT );
  lcd.begin (16,2); 
  digitalWrite ( BACKLIGHT_PIN, HIGH );
  
  Serial.begin(57600);
}

void loop()
{
  // when characters arrive over the serial port...
  if (Serial.available()) 
  {
    // wait a bit for the entire message to arrive
    delay(100);
    // clear the screen
    lcd.clear();
    // read all the available characters
    while (Serial.available() > 0) 
    {
      // display each character to the LCD
      lcd.write(Serial.read());
    }
  }
}
