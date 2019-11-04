//
// PROGMEM keyword tells the compiler "put this information into flash memory",
// instead of into RAM, where it would normally go.
//
// Using PROGMEM is a two-step procedure. After getting the data into Flash
// memory, it requires special methods (functions) to read the data from
// program memory back into RAM, so we can do something useful with it.
//

#include <SmingCore.h>
#include <stdio.h>

#include "TestProgmem.h"

const uint8_t demoRam[] = {1, 2, 3, 4, 5};
const PROGMEM uint8_t demoPgm[] = {1, 2, 3, 4, 5};

const PROGMEM char demoString[] = "Demo";
const PROGMEM char demoString2[] = "Demo";
const PROGMEM char demoFormat[] = "Demo %d";

const PROGMEM uint8_t bytes[] = {1, 2, 3, 4, 5, 6, 7, 8};
const PROGMEM uint16_t words[] = {11, 21, 31, 41, 51, 61, 71, 81};
const PROGMEM uint32_t dwords[] = {12, 120, 1200, 12000, 120000, 1200000, 12000000, 120000000};
const PROGMEM float floats[] = {13, 130, 1300, 13000, 130000, 1300000, 13000000, 130000000};

void assertEquals8(uint8_t expected, uint8_t actual)
{
	if(expected != actual) {
		Serial.print("assertEquals8: ");
		Serial.print(expected);
		Serial.print(" != ");
		Serial.println(actual);
	}
}

void assertEquals16(uint16_t expected, uint16_t actual)
{
	if(expected != actual) {
		Serial.print("assertEquals16: ");
		Serial.print(expected);
		Serial.print(" != ");
		Serial.println(actual);
	}
}

void assertEquals32(uint32_t expected, uint32_t actual)
{
	if(expected != actual) {
		Serial.print("assertEquals32: ");
		Serial.print(expected);
		Serial.print(" != ");
		Serial.println(actual);
	}
}

void assertEqualsFloat(float expected, float actual)
{
	if(expected != actual) {
		Serial.print("assertEqualsFloat: ");
		Serial.print(expected);
		Serial.print(" != ");
		Serial.println(actual);
	}
}

void assertEqualsString(String expected, String actual)
{
	if(expected != actual) {
		Serial.print("assertEqualsString: ");
		Serial.print(expected);
		Serial.print(" != ");
		Serial.println(actual);
	}
}

void testPgm()
{
	Serial.println("> Read bytes from PROGMEM:");
	for(uint8_t i = 0, b = 1; i < 8; i++, b++) {
		uint8_t d = pgm_read_byte(bytes + i);
		assertEquals8(b, d);
		Serial.print(d);
		Serial.print(" ");
	}
	Serial.println("");

	for(uint16_t i = 0, w = 11; i < 8; i++, w += 10) {
		assertEquals16(w, pgm_read_word(words + i));
	}

	for(uint32_t i = 0, dw = 12; i < 8; i++, dw *= 10) {
		assertEquals32(dw, pgm_read_dword(dwords + i));
	}

	for(uint32_t i = 0, f = 13; i < 8; i++, f *= 10) {
		assertEqualsFloat((float)f, pgm_read_float(floats + i));
	}

	Serial.println("> Copy a string from PROGMEM:");
	char buf[100];
	strcpy_P(buf, demoString);
	assertEqualsString("Demo", buf);
	assertEquals8(4, strlen_P(demoString));
	Serial.println(buf);

	strcat_P(buf, demoString);
	assertEqualsString("DemoDemo", buf);

	assertEquals8(1, strcmp_P(buf, demoString));
	assertEquals8(0, strcmp_P("Demo", demoString));

	sprintf(buf, "SuperDemo");
	assertEqualsString("Demo", strstr_P(buf, demoString));

	sprintf_P(buf, demoFormat, 1);
	assertEqualsString("Demo 1", buf);

	Serial.println("> Copy bytes from PROGMEM:");
	memcpy_P(buf, demoPgm, sizeof(demoPgm));
	for(uint8_t i = 0; i < sizeof(demoPgm); i++) {
		assertEquals8(demoRam[i], buf[i]);
		Serial.print((unsigned char)buf[i]);
		Serial.print(" ");
	}
	Serial.println("");
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 or 9600 by default

	testPgm();
	testProgmem(Serial);

	Serial.println("> ESP8266EX Memory Layout:");
	Serial.println("> 0x3FFE8000 ~ 0x3FFFBFFF - User data RAM, 80kb. Available to applications.");
	Serial.println("> 0x40200000 ~ ...        - SPI Flash.");

	Serial.print("> demoRam array address: 0x");
	Serial.print((uint32_t)demoRam, HEX);
	Serial.println(" is in the RAM");

	Serial.print("> demoPgm array address: 0x");
	Serial.print((uint32_t)demoPgm, HEX);
	Serial.println(" is in the Flash");
}
