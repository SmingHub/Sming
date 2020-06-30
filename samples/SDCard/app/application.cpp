/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 17.07.2015
Descr: SDCard/FAT file usage and write benchmark.
*/
#include <SmingCore.h>
#include <Libraries/SDCard/SDCard.h>
#include <Platform/Timers.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
#define PIN_CARD_DO 12 /* Master In Slave Out */
#define PIN_CARD_DI 13 /* Master Out Slave In */
#define PIN_CARD_CK 14 /* Serial Clock */
#define PIN_CARD_SS 15 /* Slave Select */

#define SPI_BYTE_ORDER LSBFIRST /* Sets the order the bytes are transmitted. WEMOS D1 mini requires LSBFIRST */
#define SPI_FREQ_LIMIT                                                                                                 \
	40000000 /* Sets the max frequency of SPI (init is done at a lower speed than the main communication) */

void writeToFile(const char* filename, uint32_t totalBytes, uint32_t bytesPerRound)
{
	char* buf = new char[totalBytes];
	if(buf == nullptr) {
		Serial.print("Failed to allocate heap\n");
		return;
	}

	Serial.printf("Write %d kBytes in %d Bytes increment: ", totalBytes / 1024, bytesPerRound);
	for(unsigned i = 0; i < totalBytes; i++) {
		buf[i] = (i % 10) + '0';
	}

	OneShotFastUs timer;

	FIL file;
	FRESULT fRes = f_open(&file, filename, FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		unsigned i = 0;
		do {
			uint32_t remainingBytes = totalBytes - i > bytesPerRound ? bytesPerRound : totalBytes - i;
			uint32_t bytesWritten = 0;
			f_write(&file, buf + i, remainingBytes, &bytesWritten);
			if(bytesWritten != remainingBytes) {
				Serial.printf("Only written %d bytes\n", i + bytesWritten);
				break;
			}
			i += remainingBytes;
		} while(i < totalBytes);

		f_close(&file);

		//get the time at test end
		unsigned elapsed = timer.elapsedTime();

		Serial.print((i / 1024.0f) * 1000000.0f / elapsed);
		Serial.print(" kB/s\n");
	} else {
		Serial.printf("fopen FAIL: %d \n", (unsigned int)fRes);
	}

	delete[] buf;
}

void stat_file(char* fname)
{
	FILINFO fno;
	FRESULT fr = f_stat(fname, &fno);
	switch(fr) {
	case FR_OK: {
		Serial.printf("%lu\t", fno.fsize);

		uint8_t size = 0;
		if(fno.fattrib & AM_DIR) {
			size += Serial.print("[ ");
		}

		size += Serial.print(fname);

		if(fno.fattrib & AM_DIR) {
			size += Serial.print(" ]");
		}

		Serial.print('\t');
		if(size < 8) {
			Serial.print('\t');
		}

		Serial.printf("%u/%02u/%02u, %02u:%02u\t", (fno.fdate >> 9) + 1980, (fno.fdate >> 5) & 15, fno.fdate & 31,
					  fno.ftime >> 11, (fno.ftime >> 5) & 63);
		Serial.printf("%c%c%c%c%c\n", (fno.fattrib & AM_DIR) ? 'D' : '-', (fno.fattrib & AM_RDO) ? 'R' : '-',
					  (fno.fattrib & AM_HID) ? 'H' : '-', (fno.fattrib & AM_SYS) ? 'S' : '-',
					  (fno.fattrib & AM_ARC) ? 'A' : '-');
		break;
	}

	case FR_NO_FILE:
		Serial.println("n/a");
		break;

	default:
		Serial.printf("Error(%d)\n", fr);
	}
}

/*
 * @param path Start node to be scanned (also used as work area)
 */
FRESULT ls(const char* path)
{
	DIR dir;
	FRESULT res = f_opendir(&dir, path); /* Open the directory */
	if(res == FR_OK) {
		for(;;) {
			FILINFO fno;
			res = f_readdir(&dir, &fno); /* Read a directory item */
			if(res != FR_OK || fno.fname[0] == 0) {
				break; /* Break on error or end of dir */
			}
			if(fno.fname[0] == '.') {
				continue; /* Ignore dot entry */
			}

			stat_file(fno.fname);
		}
		f_closedir(&dir);
	}

	return res;
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	//	SDCardSPI = new SPISoft(PIN_CARD_DO, PIN_CARD_DI, PIN_CARD_CK, 0);
	SDCardSPI = new SPIClass();
	SDCard_begin(PIN_CARD_SS, SPI_BYTE_ORDER, SPI_FREQ_LIMIT);

	Serial.print("\nSDCard example - !!! see code for HW setup !!! \n\n");

	/*Use of some interesting functions*/

	Serial.print("1. Listing files in the root folder:\n");
	Serial.print("Size\tName\t\tDate\t\tAttributes\n");

	ls("/");

	//2. Open file, write a few bytes, close reopen and read
	Serial.print("\n2. Open file \"test\" and write some data...\n");
	FIL file;
	FRESULT fRes = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		//you can write directly
		uint32_t actual = 0;
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

	Serial.print("\n3. Open file \"test.txt\" and read\n");
	fRes = f_open(&file, "test", FA_READ);

	if(fRes == FR_OK) {
		//read back file contents
		char buffer[64];

		uint32_t actual = 0;
		f_read(&file, buffer, sizeof(buffer), &actual);
		buffer[actual] = 0;

		Serial.printf("Read: %s \n", buffer);

		f_close(&file);
	} else {
		Serial.printf("fopen FAIL: %d \n", fRes);
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
