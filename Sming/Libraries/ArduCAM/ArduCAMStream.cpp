/*
 * ArduCAMStream.cpp
 *
 *  Created on: Feb 13, 2016
 *      Author: harry
 */

#include "ArduCAMStream.h"

// Set generic spiffs debug output call.
#ifndef ACAM_DEBUG
#define ACAM_DEBUG(...)  // Serial.printf(__VA_ARGS__)
#endif
#define ACAM_HDUMP

ArduCAMStream::ArduCAMStream(ArduCAM myCam) {
	ACAM_DEBUG("ArduCAMStream::ArduCAMStream()\n");
	myCAM = myCam;
	transfer = false;
	len = 0;
	bcount = 0;
}

ArduCAMStream::~ArduCAMStream() {
	ACAM_DEBUG("ArduCAMStream::~ArduCAMStream()\n");
}


size_t ArduCAMStream::available() {
//	int len = myCAM.read_fifo_length();
//	ACAM_DEBUG("Stream available (%d bytes)\n", len);
	return len;
}

bool ArduCAMStream::dataReady()
{
	// wait for available data
	for (int i=0; i<100; i++) {
		if (myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {

			len = myCAM.read_fifo_length();
			ACAM_DEBUG("ArduCAMStream::dataReady() -> (%d bytes Ready)\n", len);
#ifdef ACAM_HDUMP
			hdump.resetAddr();
#endif
			return true;
		}
		delay(10);
	};
	Serial.printf("ArduCAMStream::dataReady() -> no Data Available\n");
	return false;
}


bool ArduCAMStream::isFinished() {
	if (available() > 0) {
//		Serial.println("isFinished: Stream has more data!\n");
		return false;
	} else {
		ACAM_DEBUG("isFinished: available()=0 -> Stream isFinished!\n");
		return true;
	}
}


uint16_t ArduCAMStream::readMemoryBlock(char* data, int bufSize) {

	if (!transfer) {
		transfer = true;
//		HwSPI.startTX();
//		SPI.beginTransaction(SPI.SPIDefaultSettings);
		debugf("myCAM.set_fifo_burst()");
		myCAM.CS_LOW();
		myCAM.set_fifo_burst();
		// clear dummy byte -> send out LO and ignore MISO
//		HwSPI.send((uint8)0x0);
		SPI.transfer(0x0);
	}

	int bytesread = min(len, bufSize);

	ACAM_DEBUG("ArduCAMStream::readMemoryBlock [%d] (%d bytes) remaining (%d bytes)\n", bcount++, bytesread, len);

//	HwSPI.recv((uint8_t *)data, bytesread);
	SPI.transfer((unsigned char*)data, bytesread);

	len = len - bytesread;

	if (len == 0) {
		ACAM_DEBUG("ArduCAMStream::readMemoryBlock -> eof");
		transfer = false;
//		HwSPI.endTX();
		SPI.endTransaction();
	}

	myCAM.CS_HIGH();

//	hdump.print((uint8_t *)data, bytesread);

	return bytesread;

}


bool ArduCAMStream::seek(int dist) {
//	ACAM_DEBUG("ArduCAMStream::seek (%d)\n", dist);
	// we do not allow negativ seeks
	if (dist < 0)
		return false;
	// end we ignore forward seeks
	else
		return true;
};

