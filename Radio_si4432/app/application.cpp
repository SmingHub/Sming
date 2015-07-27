/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 17.07.2015
Descr: SDCard/FAT file usage and write benchmark.
*/
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/si4432/si4432.h>
#include <string.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_RADIO_DO 5	/* Master In Slave Out */
#define PIN_RADIO_DI 4	/* Master Out Slave In */
#define PIN_RADIO_CK 15	/* Serial Clock */
#define PIN_RADIO_SS 13	/* Slave Select */


void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Serial.print("\nRadio si4432 example - !!! see code for HW setup !!! \n\n");

	SPISoft *pRadioSPI = new SPISoft(PIN_RADIO_DO, PIN_RADIO_DI, PIN_RADIO_CK, PIN_RADIO_SS);

	if(pRadioSPI)
	{
		delay(300);
		Si4432 *radio = new Si4432(pRadioSPI);
		radio->init();
		radio->setBaudRate(70);
		radio->setFrequency(433);
		radio->readAll();
	}
	else Serial.print("Not enough heap\n");

}
