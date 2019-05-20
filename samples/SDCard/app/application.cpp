/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 17.07.2015
Descr: SDCard/FAT file usage and write benchmark.
*/
#include <SmingCore.h>
#include <Libraries/SDCard/SDCard.h>
#include <string.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_CARD_DO 12 /* Master In Slave Out */
#define PIN_CARD_DI 13 /* Master Out Slave In */
#define PIN_CARD_CK 14 /* Serial Clock */
#define PIN_CARD_SS 4  /* Slave Select */

void writeToFile(const char* filename, uint32_t totalBytes, uint32_t bytesPerRound)
{
	FIL file;
	FRESULT fRes;
	uint32_t t1, t2, td, byteswritten, i;
	char* buf = new char[totalBytes];

	if(!buf) {
		Serial.print("Failed to allocate heap\n");
		return;
	}

	Serial.printf("Write %d kBytes in %d Bytes increment: ", totalBytes / 1024, bytesPerRound);
	for(i = 0; i < totalBytes; i++)
		buf[i] = (i % 10) + '0';

	t1 = system_get_time();

	fRes = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		i = 0;
		do {
			uint32_t remainingBytes = totalBytes - i > bytesPerRound ? bytesPerRound : totalBytes - i;
			f_write(&file, buf + i, remainingBytes, &byteswritten);
			if(byteswritten != remainingBytes) {
				Serial.printf("Only written %d bytes\n", i + byteswritten);
				break;
			}
			i += remainingBytes;
		} while(i < totalBytes);

		f_close(&file);
	} else {
		Serial.printf("fopen FAIL: %d \n", (unsigned int)fRes);
	}
	//get the time at test end
	t2 = system_get_time();

	delete[] buf;

	Serial.print((i / 1024.0f) * 1000000.0f / (t2 - t1));
	Serial.print(" kB/s\n");
}

void stat_file(char* fname)
{
	FRESULT fr;
	FILINFO fno;
	uint8_t size = 0;
	fr = f_stat(fname, &fno);
	switch(fr) {
	case FR_OK:
		Serial.printf("%u\t", fno.fsize);

		if(fno.fattrib & AM_DIR) {
			Serial.print("[ ");
			size += 2;
		}

		Serial.print(fname);
		size += strlen(fname);

		if(fno.fattrib & AM_DIR) {
			Serial.print(" ]");
			size += 2;
		}

		Serial.print("\t");
		if(size < 8)
			Serial.print("\t");

		Serial.printf("%u/%02u/%02u, %02u:%02u\t", (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
					  fno.ftime >> 11, fno.ftime >> 5 & 63);
		Serial.printf("%c%c%c%c%c\n", (fno.fattrib & AM_DIR) ? 'D' : '-', (fno.fattrib & AM_RDO) ? 'R' : '-',
					  (fno.fattrib & AM_HID) ? 'H' : '-', (fno.fattrib & AM_SYS) ? 'S' : '-',
					  (fno.fattrib & AM_ARC) ? 'A' : '-');
		break;

	case FR_NO_FILE:
		Serial.printf("n/a\n");
		break;

	default:
		Serial.printf("Error(%d)\n", fr);
	}
}

FRESULT ls(const char* path /* Start node to be scanned (also used as work area) */
)
{
	FRESULT res;
	FILINFO fno;
	DIR dir;
	char* fn; /* This function assumes non-Unicode configuration */

	res = f_opendir(&dir, path); /* Open the directory */
	if(res == FR_OK) {
		int i = strlen(path);
		for(;;) {
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if(res != FR_OK || fno.fname[0] == 0)
				break; /* Break on error or end of dir */
			if(fno.fname[0] == '.')
				continue; /* Ignore dot entry */

			stat_file(fno.fname);
		}
		f_closedir(&dir);
	}

	return res;
}

void init()
{
	FIL file;
	FRESULT fRes;
	uint32_t actual;

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	//	SDCardSPI = new SPISoft(PIN_CARD_DO, PIN_CARD_DI, PIN_CARD_CK, 0);
	SDCardSPI = new SPIClass();
	SDCard_begin(PIN_CARD_SS);
	// overwrite default SPI Speed
	SDCardSPI->beginTransaction(SPISettings(40000000, MSBFIRST, SPI_MODE0));

	Serial.print("\nSDCard example - !!! see code for HW setup !!! \n\n");

	/*Use of some interesting functions*/

	Serial.print("1. Listing files in the root folder:\n");
	Serial.print("Size\tName\t\tDate\t\tAttributes\n");

	ls("/");

	//2. Open file, write a few bytes, close reopen and read
	Serial.print("\n2. Open file \"test\" and write some data...\n");
	fRes = f_open(&file, "test", FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		//you can write directly
		f_write(&file, "hello ", 5, &actual);

		//or using printf for convenience
		f_printf(&file, " has %d letters\r\n", actual);

		if(actual != 5) {
			Serial.printf("Only written %d bytes\n", actual);
		}
		f_close(&file);
	} else {
		Serial.printf("fopen FAIL: %d \n", (unsigned int)fRes);
	}

	Serial.print("\n3. Open file \"test\" and read\n");
	fRes = f_open(&file, "test", FA_READ);

	if(fRes == FR_OK) {
		//read back file contents
		char buffer[64];

		f_read(&file, buffer, sizeof(buffer), &actual);
		buffer[actual] = 0;

		Serial.printf("Read: %s \n", buffer);

		f_close(&file);
	} else {
		Serial.printf("fopen FAIL: %d \n", (unsigned int)fRes);
	}

	Serial.print("\n4. Write speed benchamark:\n");
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
