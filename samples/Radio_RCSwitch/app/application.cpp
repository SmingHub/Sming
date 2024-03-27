#include <SmingCore.h>
#include <Libraries/RCSwitch/RCSwitch.h>

#define LED_PIN 2 // GPIO2

Timer sendTimer;
Timer receiveTimer;

RCSwitch mySwitch = RCSwitch();

void sendRF()
{
	mySwitch.send(5393, 24);
	Serial.println(_F("RF command successful sent"));
}

void receiveRF()
{
	if(mySwitch.available()) {
		if(mySwitch.getReceivedValue() == 0) {
			Serial.print(_F("Unknown encoding"));
		} else {
			Serial << _F("Received ") << mySwitch.getReceivedValue() << " / " << mySwitch.getReceivedBitlength()
				   << " bit, Protocol: " << mySwitch.getReceivedProtocol() << endl;
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

	sendTimer.initializeMs<1000>(sendRF).start();
	receiveTimer.initializeMs<20>(receiveRF).start();
}
