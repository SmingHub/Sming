
// Edge Test For use with the DANGER SHIELD

// This code turns a LED on and incraments number displayed on the 7-segment display
//  whenever either PB1 or PB2 is pressed. PB1 used the debounce library and PB2 does
//  not. PB1 will cleanly increment the displayed value. PB2 will not alway increment
//  the display because the switch is not debounced.
//
// The  bounce library also detects the rising or falling edge of the input.
//  This is often called a one-shot and is usefull if you want something to only 
//  happen once when a button is pressed or released. The risingEdge method is true
//  for only one scan when the input goes from off-to-on. The fallingEdge method is
//  true for only one scan when the input goes from on-to-off. Even if you are not
//  using the debounce feature the the library you need to call the update method
//  every scan for the edge detection to work.
//
// Notes:
//  The buttons on the Danger Shield pull the input to ground when they are pressed,
//  so they act like normally closed switches. If order to make them work like one
//  would expect, he tests for the button states are reversed. This also reversesd
//  the rising edge test on button1 to a falling edge test.
//
//  Since PB2 is not debounced, the edge detection is done directly in the sketch.
//   


#include <Bounce.h>

// This code turns a led on/off through a debounced button
// Build the circuit indicated here: http://arduino.cc/en/Tutorial/Button

#define BUTTON1 10
#define BUTTON2 11
#define BUTTON3 12

#define BUZZER 3

#define LED1 5
#define LED2 6

#define LATCH 7
#define CLOCK 8
#define DATA 4


// Shift register bit values to display 0-9 on the seven-segment display
const byte ledCharSet[16] = {
  B00111111, B00000110, B01011011, B01001111, B01100110, 
  B01101101, B01111101, B00000111, B01111111, B01101111,
  B01110111, // a
  B01111100, // b
  B00111001, // c
  B01011110, // d
  B01111001, // e
  B01110001  // f
};

byte pressCount= 0;
bool lastButton2;

// Instantiate a Bounce object with a 5 millisecond debounce time
Bounce bouncer1 = Bounce(BUTTON1, 5); 

void setup() {
  pinMode(BUTTON1, INPUT);
  pinMode(LED1, OUTPUT);
  
  pinMode(BUTTON2, INPUT);
  pinMode(LED2, OUTPUT);
  
  pinMode(BUTTON3, INPUT);
  pinMode(BUZZER, OUTPUT);
  
  pinMode(LATCH, OUTPUT);
  pinMode(CLOCK, OUTPUT);
  pinMode(DATA, OUTPUT);
  
  lastButton2 = digitalRead(BUTTON2);

  displayDigit(255, true);
}


void loop() {

  // Update debouncer 
  boolean button1Changed = bouncer1.update();

  // Turn on or off the LED
  if ( !bouncer1.read()) {
    digitalWrite(LED1, HIGH);
  } else {
    digitalWrite(LED1, LOW);
  }
    
  int buttonValue2 = digitalRead(BUTTON2);
  if (buttonValue2 == LOW) {
    digitalWrite(LED2, HIGH);
  } else {
    digitalWrite(LED2, LOW);
  }

  if ( bouncer1.fallingEdge() ||          // rising edge of button 1
      (lastButton2 && !buttonValue2)) {   // rising edge of button 2
    displayDigit(pressCount, false); 
    pressCount++;
    if (pressCount > 15) { pressCount = 0; }
  }
 
  lastButton2 = digitalRead(BUTTON2);

}

void displayDigit(byte value, boolean dp) {
  byte shiftData = 0;
  if (value <= 15) { shiftData = ledCharSet[value]; }
  if (dp) { shiftData |= B10000000; }
  digitalWrite(LATCH,LOW);
  shiftOut(DATA, CLOCK, MSBFIRST, ~shiftData);
  digitalWrite(LATCH,HIGH);
}
