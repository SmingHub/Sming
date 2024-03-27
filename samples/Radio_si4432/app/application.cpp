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

Timer procTimer;
Si4432* radio = nullptr;
SPISoft* pRadioSPI = nullptr;

#define PING_PERIOD_MS 2000
#define PING_WAIT_PONG_MS 100
unsigned long lastPingTime;

void loopListen()
{
	const byte* ack = (const byte*)"OK"; //{ 0x01, 0x3, 0x11, 0x13 };
	const byte* ping = (const byte*)"PING";
	byte payLoad[64] = {0};
	byte len = 0;

	//1. Ping from time to time, and wait for incoming response
	if(millis() - lastPingTime > PING_PERIOD_MS) {
		lastPingTime = millis();

		Serial.print(_F("Ping -> "));
		if(!radio->sendPacket(strlen((const char*)ping), ping, true, PING_WAIT_PONG_MS, &len, payLoad)) {
			Serial.println(" ERR!");
		} else {
			Serial.println(_F(" SENT!"));
			Serial << "SYNC RX (" << len << "): ";
			Serial.write(payLoad, len);
			Serial.println();
		}
	}

	//2. Listen for any other incoming packet
	bool pkg = radio->isPacketReceived();

	if(pkg) {
		radio->getPacketReceived(&len, payLoad);
		Serial << _F("ASYNC RX (") << len << "): ";
		Serial.write(payLoad, len);
		Serial.println();

		Serial.print(_F("Response -> "));
		if(!radio->sendPacket(strlen((const char*)ack), ack)) {
			Serial.println(_F("ERR!"));
		} else {
			Serial.println(_F("SENT!"));
		}

		radio->startListening(); // restart the listening.
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

	Serial.systemDebugOutput(true); //Allow debug output to serial

	Serial.println(_F("\nRadio si4432 example - !!! see code for HW setup !!! \n"));

	pRadioSPI = new SPISoft(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS);

	if(pRadioSPI != nullptr) {
		radio = new Si4432(pRadioSPI);
	}

	if(radio == nullptr) {
		Serial.println(_F("Error: Not enough heap."));
		return;
	}

	delay(100);

	//initialise radio with default settings
	radio->init();

	//explicitly set baudrate and channel
	radio->setBaudRateFast(eBaud_38k4);
	radio->setChannel(0);

	//dump the register configuration to console
	radio->readAll();

	//start listening for incoming packets
	Serial.println("Listening...");
	radio->startListening();

	lastPingTime = millis();

	//start listen loop
	procTimer.initializeMs<10>(loopListen).start();
}
