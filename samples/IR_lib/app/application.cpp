/***************************************************
  A simple example on how to use the IR library
 ****************************************************/

#include <SmingCore.h>
#include <Libraries/IR/src/IRrecv.h>
#include <Libraries/IR/src/IRsend.h>
#include <Libraries/IR/src/IRutils.h>

#define IR_RECV_PIN 12 // GPIO12
#define IR_SEND_PIN 5  // GPIO5

Timer irTimer;
IRrecv irrecv(IR_RECV_PIN);
IRsend irsend(IR_SEND_PIN);

void receiveIR()
{
	irTimer.stop();
	decode_results dresults;
	dresults.decode_type = UNUSED;
	if(irrecv.decode(&dresults)) {
		Serial.println(resultToHumanReadableBasic(&dresults)); // Output the results as source code
		Serial.println(resultToTimingInfo(&dresults));
	}
	if(dresults.decode_type > UNUSED) {
		Serial.println("Send IR Code");
		irsend.send(dresults.decode_type, dresults.value, dresults.bits);
	}
	irTimer.start();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.println("Setting up...");
	irrecv.enableIRIn(); // Start the receiver
	irTimer.initializeMs(200, receiveIR).start();
	Serial.println("Ready...");
}
