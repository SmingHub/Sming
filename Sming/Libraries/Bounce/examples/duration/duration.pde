#include <Bounce.h>

/*
This example uses a mercury tilt switch to detect the orientation 
of a robot. If you turn the bot over, it falls asleep to conserve 
battery life.  If you have ever used a tilt switch before,
you know they can be very "noisy" - they jump around a lot!  
The bounce library can handle sensing when a noisy switch has 
"settled down"  with the "duration" method that tells you how long 
the pin has been in the current state.
*/

const uint8_t ORIENTATION_PIN = 16;

Bounce orientation = Bounce( ORIENTATION_PIN, 50 );

int awake; // Are we awake based on our current orientation?

void setup() {

  pinMode( ORIENTATION_PIN, INPUT );
  digitalWrite( ORIENTATION_PIN, HIGH ); // Activate Pull-Up

  Serial.begin(19200);
  Serial.println( "Orientation Test " );
  Serial.println();

  awake = orientation.read();

  Serial.println( awake ? "Awake!" : "Asleep!" );

}

unsigned long lastKnock = 0;

void loop() {

  orientation.update();

  // has our orientation changed for more the 1/2 a second?
  if ( orientation.read() != awake && orientation.duration() > 500 ) {

    awake = orientation.read();

    if ( awake ) {

      Serial.println( "Waking Up!" );
      // Do Something Here...

    }
    else {

      Serial.println( "Falling Asleep" );
      // Do Something Here...

    }
  }

}
