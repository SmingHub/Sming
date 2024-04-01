/*
Modified by: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 15.08.2015
Descr: Example application for radio module Si4432 aka RF22 driver
Link: http://www.electrodragon.com/w/SI4432_433M-Wireless_Transceiver_Module_%281.5KM_Range,_Shield-Protected%29
*/

#include <SmingCore.h>
#include <Libraries/si4432/si4432.h>
#include <string.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_RADIO_DO 5  /* Master In Slave Out */
#define PIN_RADIO_DI 4  /* Master Out Slave In */
#define PIN_RADIO_CK 15 /* Serial Clock */
#define PIN_RADIO_SS 13 /* Slave Select */

namespace
{
SimpleTimer procTimer;
SPISoft radioSPI(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS);
Si4432 radio(&radioSPI);

#define PING_PERIOD_MS 2000
#define PING_WAIT_PONG_MS 100

PeriodicFastMs pingTimer;

void loopListen()
{
	const char* ack = "OK"; //{ 0x01, 0x3, 0x11, 0x13 };
	const char* ping = "PING";
	byte payLoad[64] = {0};
	byte len = 0;

	//1. Ping from time to time, and wait for incoming response
	if(pingTimer.expired()) {
		Serial.print(_F("Ping -> "));
		if(!radio.sendPacket(strlen(ping), reinterpret_cast<const uint8_t*>(ping), true, PING_WAIT_PONG_MS, &len,
							 payLoad)) {
			Serial.println(" ERR!");
		} else {
			Serial.println(_F(" SENT!"));
			Serial << "SYNC RX (" << len << "): ";
			Serial.write(payLoad, len);
			Serial.println();
		}
	}

	//2. Listen for any other incoming packet
	bool pkg = radio.isPacketReceived();

	if(pkg) {
		radio.getPacketReceived(&len, payLoad);
		Serial << _F("ASYNC RX (") << len << "): ";
		Serial.write(payLoad, len);
		Serial.println();

		Serial.print(_F("Response -> "));
		if(!radio.sendPacket(strlen(ack), reinterpret_cast<const uint8_t*>(ack))) {
			Serial.println(_F("ERR!"));
		} else {
			Serial.println(_F("SENT!"));
		}

		radio.startListening(); // restart the listening.
	}
}

} // namespace

void init()
{
#ifdef ARCH_HOST
	setDigitalHooks(nullptr);
#endif

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	Serial.systemDebugOutput(true); //Allow debug output to serial

	Serial.println(_F("\r\nRadio si4432 example - !!! see code for HW setup !!!\r\n"));

	// initialise radio with default settings
	radio.init();

	// explicitly set baudrate and channel
	radio.setBaudRateFast(eBaud_38k4);
	radio.setChannel(0);

	// dump the register configuration to console
	radio.readAll();

	// start listening for incoming packets
	Serial.println("Listening...");
	radio.startListening();

	pingTimer.reset(PING_PERIOD_MS);

	// start listen loop
	procTimer.initializeMs<10>(loopListen).start();
}
