/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 17.07.2015
Descr: SDCard/FAT file usage and write benchmark.
*/
#include <user_config.h>
#include <SmingCore/SmingCore.h>
#include <Libraries/SDCard/SDCard.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_CARD_DO 5	/* Master In Slave Out */
#define PIN_CARD_DI 4	/* Master Out Slave In */
#define PIN_CARD_CK 15	/* Serial Clock */
#define PIN_CARD_SS 12	/* Slave Select */

void writeToFile(const char* filename, uint32_t totalBytes, uint32_t bytesPerRound)
{
	FIL file;
	FRESULT fRes;
	uint32_t t1, t2, td, byteswritten, i, remainingBytes;
	char *buf = new char[totalBytes];

	if(!buf)
	{
		Serial.print("Failed to allocate heap\n");
		return;
	}

	Serial.printf("Write %d kBytes in %d Bytes increment: ", totalBytes/1024, bytesPerRound);
	for(i = 0; i < totalBytes; i++)
		buf[i] = (i % 10) + '0';

	t1 = system_get_time();

	fRes = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);

	if (fRes == FR_OK)
	{
		i = 0;
		do
		{
			remainingBytes = totalBytes - i > bytesPerRound ? bytesPerRound : totalBytes - i;
			f_write(&file, buf + i, remainingBytes, &byteswritten);
			if (byteswritten != remainingBytes)
			{
				Serial.printf("Only written %d bytes\n", i + byteswritten);
				break;
			}
			i += remainingBytes;
		}
		while(i < totalBytes);

		f_close(&file);
	}
	else
	{
		Serial.printf("fopen FAIL: %d \n", (unsigned int)fRes);
	}
	//get the time at test end
	t2 = system_get_time();

	delete buf;

	Serial.print((i/1024.0f)*1000000.0f / (t2-t1));
	Serial.print(" kB/s\n");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	SDCard = new SDCardClass(PIN_CARD_DO, PIN_CARD_DI, PIN_CARD_CK, PIN_CARD_SS);
	SDCard->begin();

	Serial.print("\nSDCard benchmark - see code for HW setup\n\n");

	writeToFile("f1.txt", 1024, 1);

	writeToFile("f2.txt", 1024, 64);

	writeToFile("f3.txt", 1024, 128);

	writeToFile("f4.txt", 1024, 512);

	writeToFile("f5.txt", 1024, 1024);

	writeToFile("f6.txt", 4096, 1024);

	writeToFile("f7.txt", 8192, 512);

	writeToFile("f8.txt", 8192, 1024);

	writeToFile("f9.txt", 8192, 8192);
}
