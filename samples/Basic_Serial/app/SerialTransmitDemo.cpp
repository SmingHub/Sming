/*
 * SerialTransmitDemo.cpp
 */

#include "SerialTransmitDemo.h"

const unsigned DEMO_BUFFER_SIZE = 32;

/* SerialStreamTransmitDemo */

void SerialStreamTransmitDemo::sendDataChunk()
{
	char buffer[DEMO_BUFFER_SIZE];
	size_t count = stream->readMemoryBlock(buffer, sizeof(buffer));
	if(count > 0) {
		serial->write(buffer, count);
		stream->seek(count);
	}

	if(count != sizeof(buffer)) {
		Serial.println(_F("Finished sending stream to serial port"));

		// All done, delete ourselves
		delete this;
	}
}

/* SerialFlashMemoryTransmitDemo */

void SerialFlashMemoryTransmitDemo::sendDataChunk()
{
	char buffer[DEMO_BUFFER_SIZE];
	unsigned count = min(sizeof(buffer), data->length() - readPos);
	memcpy_P(buffer, data->flashData + readPos, count);
	serial->write(buffer, count);
	readPos += count;

	if(count != sizeof(buffer)) {
		Serial.println(_F("Finished sending flash data to serial port"));

		// All done, delete ourselves
		delete this;
	}
}
