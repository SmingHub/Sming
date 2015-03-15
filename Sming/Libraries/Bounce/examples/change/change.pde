#include <Bounce.h>
#define BUTTON 2
#define LED 13

int ledValue = LOW;

// This example changes the state of the LED everytime the button is pushed
// Build the circuit indicated here: http://arduino.cc/en/Tutorial/Button


Bounce bouncer = Bounce( BUTTON, 5 ); 

void setup() {
  pinMode(BUTTON,INPUT);
  pinMode(LED,OUTPUT);
}

void loop() {

   if ( bouncer.update() ) {
     if ( bouncer.read() == HIGH) {
       if ( ledValue == LOW ) {
         ledValue = HIGH;
       } else {
         ledValue = LOW;
       }
       digitalWrite(LED,ledValue);
     }
   }
}

