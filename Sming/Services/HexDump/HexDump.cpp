/*
 * HexDump.cpp
 *
 *  Created on: Feb 14, 2016
 *      Author: harry
 */

#include "HexDump.h"

HexDump::HexDump()
{
	addr = 0;
}

HexDump::~HexDump()
{
	// TODO Auto-generated destructor stub
}

void HexDump::print(unsigned char* data, int len)
{
	Serial.printf("Data: (%d Bytes)\n", len);
	int idx = 0;
	while(len - idx > 16) {
		pritln((char*)&data[idx], 16);
		idx += 16;
	}
	pritln((char*)&data[idx], len - idx);
}

void HexDump::pritln(char* buf, int len)
{
	// print address
	Serial.printf("%06X: ", addr);

	// Show the hex codes
	for(int i = 0; i < 16; i++) {
		if(i % 8 == 0) {
			Serial.printf(" ");
		}
		if(i < len) {
			Serial.printf("%02X ", buf[i]);
		} else {
			Serial.printf("   ");
		}
	}
	// Show printable characters
	Serial.printf("  ");
	for(int i = 0; i < len; i++) {
		Serial.printf("%c", isprint(buf[i]) ? buf[i] : '.');
	}

	Serial.printf("\n");
	addr += len;
}

void HexDump::resetAddr()
{
	addr = 0;
}
