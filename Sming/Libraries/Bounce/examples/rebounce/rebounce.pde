#include <Bounce.h>

// As long as the button is held down, the LED will blink
// Build the circuit indicated here: http://arduino.cc/en/Tutorial/Button

#define BUTTON 2
#define LED 13

// A variable to store the current LED state
int ledState = LOW;

// Instantiate a Bounce object with a 5 millisecond debounce time
Bounce bouncer = Bounce( BUTTON,5 ); 

void setup() {
  pinMode(BUTTON,INPUT);
  pinMode(LED,OUTPUT);
}

void loop() {
  
 // Update and monitor a change of input
  if ( bouncer.update() ) {
    
    // Get the state of the button
   int value = bouncer.read();
 
   // Toggle the LED if the button is held
   if ( value == HIGH) {
     // Make the button retrigger in 500 milliseconds
     bouncer.rebounce(500);
     if ( ledState == LOW ) {
       ledState = HIGH;
     } else {
       ledState = LOW;
     }
   } else {
     ledState = LOW;
   }
 
    digitalWrite(LED, ledState );
    
  }
 
 
 
}

