/*
Author: (github.com/)ADiea
Project: Sming for ESP8266 - https://github.com/anakod/Sming
License: MIT
Date: 17.07.2015
Descr: SDCard/FAT file usage and write benchmark.
*/
#include <SmingCore.h>
#include <Libraries/SDCard/SDCard.h>

/*(!) Warning on some hardware versions (ESP07, maybe ESP12)
 * 		pins GPIO4 and GPIO5 are swapped !*/
// ** Definitions for software SPI ONLY **
#define PIN_CARD_DO 12 /* Master In Slave Out */
#define PIN_CARD_DI 13 /* Master Out Slave In */
#define PIN_CARD_CK 14 /* Serial Clock */

// Chip selects independent of SPI controller in use
#ifdef ARCH_ESP32
#define PIN_CARD_SS 21
#else
// Esp8266 cannot use GPIO15 as this affects boot mode
#define PIN_CARD_SS 5
#endif

/* Sets the max frequency of SPI (init is done at a lower speed than the main communication) */
#define SPI_FREQ_LIMIT 2000000

namespace
{
void writeToFile(const String& filename, uint32_t totalBytes, uint32_t bytesPerRound)
{
	char* buf = new char[totalBytes];
	if(buf == nullptr) {
		Serial.println(_F("Failed to allocate heap"));
		return;
	}

	Serial << _F("Write ") << totalBytes / 1024 << _F(" kBytes in ") << bytesPerRound << _F(" Bytes increment:")
		   << endl;
	for(unsigned i = 0; i < totalBytes; i++) {
		buf[i] = (i % 10) + '0';
	}

	OneShotFastUs timer;

	FIL file;
	FRESULT fRes = f_open(&file, filename.c_str(), FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		unsigned i = 0;
		do {
			uint32_t remainingBytes = totalBytes - i > bytesPerRound ? bytesPerRound : totalBytes - i;
			uint32_t bytesWritten = 0;
			f_write(&file, buf + i, remainingBytes, &bytesWritten);
			if(bytesWritten != remainingBytes) {
				Serial << _F("Only written ") << i + bytesWritten << " bytes" << endl;
				break;
			}
			i += bytesWritten;
		} while(i < totalBytes);

		f_close(&file);

		//get the time at test end
		unsigned elapsed = timer.elapsedTime();

		Serial << (i / 1024.0f) * 1000000.0f / elapsed << _F(" kB/s") << endl;
	} else {
		Serial << _F("fopen FAIL: ") << fRes << endl;
	}

	delete[] buf;
}

void stat_file(char* fname)
{
	FILINFO fno;
	FRESULT fr = f_stat(fname, &fno);
	switch(fr) {
	case FR_OK: {
		Serial << fno.fsize << '\t';

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

		Serial << (fno.fdate >> 9) + 1980 << '/' << String((fno.fdate >> 5) & 15, DEC, 2) << '/'
			   << String(fno.fdate & 31, DEC, 2) << ", " << String(fno.ftime >> 11, DEC, 2) << ':'
			   << String((fno.ftime >> 5) & 63, DEC, 2) << '\t' << ((fno.fattrib & AM_DIR) ? 'D' : '-')
			   << ((fno.fattrib & AM_RDO) ? 'R' : '-') << ((fno.fattrib & AM_HID) ? 'H' : '-')
			   << ((fno.fattrib & AM_SYS) ? 'S' : '-') << ((fno.fattrib & AM_ARC) ? 'A' : '-') << endl;
		break;
	}

	case FR_NO_FILE:
		Serial.println("n/a");
		break;

	default:
		Serial << _F("Error ") << fr << endl;
	}
}

/*
 * @param path Start node to be scanned (also used as work area)
 */
FRESULT ls(const char* path)
{
	Serial.println(_F("Size\tName\t\tDate\t\tAttributes"));

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

void listFiles()
{
	Serial.println(_F("1. Listing files in the root folder"));
	ls("/");
}

void readWriteTest()
{
	//2. Open file, write a few bytes, close reopen and read
	Serial.println(_F("2. Open file \"test.txt\" and write some data..."));
	FIL file;
	FRESULT fRes = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);

	if(fRes == FR_OK) {
		//you can write directly
		uint32_t actual = 0;
		f_write(&file, "hello ", 5, &actual);

		//or using printf for convenience
		f_printf(&file, _F(" has %d letters\r\n"), actual);

		if(actual != 5) {
			Serial << _F("Only written ") << actual << _F(" bytes") << endl;
		}
		f_close(&file);
	} else {
		Serial << _F("fopen FAIL: ") << fRes << endl;
	}
}

void readTest()
{
	Serial.println(_F("3. Open file \"test.txt\" and read"));

	FIL file;
	FRESULT fRes = f_open(&file, "test.txt", FA_READ);

	if(fRes == FR_OK) {
		//read back file contents
		char buffer[64];

		uint32_t actual = 0;
		f_read(&file, buffer, sizeof(buffer), &actual);
		buffer[actual] = 0;

		Serial << _F("Read: ") << buffer << endl;

		f_close(&file);
	} else {
		Serial << _F("fopen FAIL: ") << fRes << endl;
	}
}

bool speedTest(unsigned num)
{
	struct Test {
		uint32_t totalBytes;
		uint32_t bytesPerRound;
	};

	static const Test tests[] PROGMEM{
		{1024, 1},	//
		{1024, 64},   //
		{1024, 128},  //
		{1024, 512},  //
		{1024, 1024}, //
		{4096, 1024}, //
		{8192, 512},  //
		{8192, 1024}, //
		{8192, 8192}, //

	};

	if(num >= ARRAY_SIZE(tests)) {
		return false;
	}

	Serial << "4." << num + 1 << _F(": Write speed benchmark") << endl;

	auto& test = tests[num];
	String filename;
	filename = 'f';
	filename += num + 1;
	filename += ".txt";
	writeToFile(filename, test.totalBytes, test.bytesPerRound);
	return true;
}

bool sdInit()
{
	//  select between hardware SPI or software SPI. Comment out one or the other
	//  SDCardSPI = new SPISoft(PIN_CARD_DO, PIN_CARD_DI, PIN_CARD_CK, 0);
	SDCardSPI = &SPI;
	if(!SDCard_begin(PIN_CARD_SS, SPI_FREQ_LIMIT)) {
		Serial.println(_F("SPI init failed"));
		return false;
	}

	return true;
}

void runTest(uint32_t state = 0)
{
	/* Use of some interesting functions */
	switch(state) {
	case 0:
		break;
	case 1:
		if(!sdInit()) {
			return;
		}
		break;
	case 2:
		listFiles();
		break;
	case 3:
		readWriteTest();
		break;
	case 4:
		readTest();
		break;
	default:
		if(!speedTest(state - 5)) {
			Serial.println(_F("End of tests"));
			return;
		}
	}
	Serial.println();
	System.queueCallback(runTest, state + 1);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	Serial.print(_F("\r\nSDCard example - !!! see code for HW setup !!! \r\n\n"));
	runTest();
}
