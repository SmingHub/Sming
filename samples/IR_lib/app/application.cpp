 /***************************************************
  A simple example on how to use the IR library
 ****************************************************/

#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/IR/IRremote.h>
#include <Libraries/IR/IRremoteInt.h>

#define IR_PIN 12 // GPIO12

Timer irTimer;
decode_results dresults;
IRrecv irrecv(IR_PIN);
IRsend irsend(IR_PIN);

void receiveIR()
{
	if(irrecv.decode(&dresults)==DECODED){
		irTimer.stop();
		unsigned int * sendbuff = new unsigned int[dresults.rawlen-1];
		for(int i=0; i<dresults.rawlen-1; i++){
			sendbuff[i]=dresults.rawbuf[i+1]*50;
		}
		irsend.sendNEC(dresults.value, dresults.bits);
		Serial.println("Sent IR Code");
		irrecv.enableIRIn();
		irTimer.start();
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.println("Setting up...");
	irrecv.blink13(1);
	irrecv.enableIRIn(); // Start the receiver
	irTimer.initializeMs(1000, receiveIR).start();
	Serial.println("Ready...");
}
