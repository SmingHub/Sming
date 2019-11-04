#include <SmingCore.h>
#include <Libraries/RCSwitch/RCSwitch.h>

#define LED_PIN 2 // GPIO2

Timer sendTimer;
Timer receiveTimer;

RCSwitch mySwitch = RCSwitch();

void sendRF()
{
	mySwitch.send(5393, 24);
	Serial.println("RF command successful sent");
}

void receiveRF()
{
	if(mySwitch.available()) {
		if(mySwitch.getReceivedValue() == 0) {
			Serial.print("Unknown encoding");
		} else {
			Serial.print("Received ");
			Serial.print(mySwitch.getReceivedValue());
			Serial.print(" / ");
			Serial.print(mySwitch.getReceivedBitlength());
			Serial.print("bit ");
			Serial.print("Protocol: ");
			Serial.println(mySwitch.getReceivedProtocol());
		}

		mySwitch.resetAvailable();
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	mySwitch.enableTransmit(5); // pin GPIO5 - transmit
	mySwitch.enableReceive(4);  // pin GPIO4  - receive

	// Optional set pulse length.
	//mySwitch.setPulseLength(240);
	// Optional set protocol (default is 1, will work for most outlets)
	// mySwitch.setProtocol(2);

	sendTimer.initializeMs(1000, sendRF).start();
	receiveTimer.initializeMs(20, receiveRF).start();
}
