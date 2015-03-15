#include <Wire.h>
#include <LiquidCrystal_SR.h>

LiquidCrystal_SR lcd(8,7,TWO_WIRE);
//                   | |
//                   | \-- Clock Pin
//                   \---- Data/Enable Pin

// Creat a set of new characters
byte armsUp[8] = {0b00100,0b01010,0b00100,0b10101,0b01110,0b00100,0b00100,0b01010};
byte armsDown[8] = {0b00100,0b01010,0b00100,0b00100,0b01110,0b10101,0b00100,0b01010};

void setup(){

  lcd.begin(16,2);               // initialize the lcd

  lcd.createChar (0, armsUp);    // load character to the LCD
  lcd.createChar (1, armsDown);    // load character to the LCD

  lcd.home ();                   // go home
  lcd.print("LiquidCrystal_SR");
}

void loop(){
  // Do a little animation
  for(int i = 0; i <= 15; i++) showHappyGuy(i);
  for(int i = 15; i >= 0; i--) showHappyGuy(i);
}

void showHappyGuy(int pos){
  lcd.setCursor ( pos, 1 ); // go to position
  lcd.print(char(random(0,2))); // show one of the two custom characters
  delay(150); // wait so it can be seen
  lcd.setCursor ( pos, 1 ); // go to position again
  lcd.print(" "); // delete character
}