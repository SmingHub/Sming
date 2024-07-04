/*
 * HexDump.h
 *
 *  Created on: Feb 14, 2016
 *      Author: harry
 */

#pragma once

#include "Arduino.h"

class HexDump
{
public:
	virtual ~HexDump() = default;

	void print(unsigned char* data, int len);
	void resetAddr();

private:
	void pritln(char* buf, int len);
	int addr = 0;
};
