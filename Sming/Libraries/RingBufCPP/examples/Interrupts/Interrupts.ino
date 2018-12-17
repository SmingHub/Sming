/*
  RingBufInterruptsExample.ino - Interrupt example, to demonstrate the power of a ring buffer with asynchronous io
  Created by D. Aaron Wisner (daw268@cornell.edu)
  January 17, 2015.
  Released into the public domain.

  Trigger an interrupt by toggling EVENT_PIN, this will push an Event struct into the buffer
  Print the contents of the buffer by sending anything over Serial Monitor
*/
#include <RingBufCPP.h>

// MUST BE AN INTTERUPT COMPATIBLE PIN
#define EVENT_PIN 3
#define MAX_NUM_ELEMENTS 10

struct Event
{
  int index;
  unsigned char pinState;
  unsigned long timestamp;
};

// Declare as volatile, since modofied in ISR
volatile unsigned int index = 0;

// Stack allocate the buffer to hold event structs
RingBufCPP<struct Event, MAX_NUM_ELEMENTS> buf;


void setup() {
  Serial.begin(9600);

  // Attach the interrupt service routine
  attachInterrupt(digitalPinToInterrupt(EVENT_PIN), isr_event, CHANGE);


  Serial.println("Toggle the pin state of pin EVENT_PIN, to fire an interrupt");
  Serial.println("Send anything over serial to dump the contents of the buffer");
  Serial.println();
}

void loop() {
  // Print the number of elements
  Serial.print("There are: ");
  Serial.print(buf.numElements());
  Serial.println(" element(s) in the ring buffer");

  // Check if empty
  if (buf.isEmpty())
  {
    Serial.println("The ring buffer is currently empty");
  }
  else if (buf.isFull())
  {
    Serial.println("The ring buffer is currently full");
  }

  // If any message sent through serial monitor, dump buffer
  if (Serial.available() > 0)
  {
    // Print contents of buffer
    print_buf_contents();

    //empty serial buffer
    while (Serial.read() > 0);
  }

  Serial.println();

  delay(2000);
}


// This interrupt serve routine function is called whenever an interrupt fires on EVENT_PIN
// An Event Struct will be appened to the buffer
void isr_event()
{
  struct Event e;
  e.index = index++;
  e.pinState = digitalRead(EVENT_PIN);
  e.timestamp = millis();
  // Add it to the buffer
  buf.add(e);
}

// Print the buffer's contents then empty it
void print_buf_contents()
{
  struct Event e;

  Serial.println("\n______Dumping contents of ring buffer_______");

  // Keep looping until pull() returns NULL
  while (buf.pull(&e))
  {
    //
    Serial.print("Event index: ");
    Serial.println(e.index);

    Serial.print("Pin state: ");
    Serial.println(e.pinState);

    Serial.print("Timestamp (ms): ");
    Serial.println(e.timestamp);

    Serial.println();
  }

  Serial.println("______Done dumping contents_______");

}
