/*
 * SerialTransmitDemo.cpp
 */

#include "SerialTransmitDemo.h"

void SerialTransmitDemo::sendDataChunk()
{
	if(serial.copyFrom(stream, chunkSize) == 0) {
		Serial.println(_F("Finished sending stream to serial port"));

		// All done, delete ourselves
		delete this;
	}
}
