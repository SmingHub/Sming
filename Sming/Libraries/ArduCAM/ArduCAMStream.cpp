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

#define BMPIMAGEOFFSET 66

const char bmp_header[BMPIMAGEOFFSET] =
{
  0x42, 0x4D, 0x36, 0x58, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x28, 0x00,
  0x00, 0x00, 0x40, 0x01, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x01, 0x00, 0x10, 0x00, 0x03, 0x00,
  0x00, 0x00, 0x00, 0x58, 0x02, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0xC4, 0x0E, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0xE0, 0x07, 0x00, 0x00, 0x1F, 0x00,
  0x00, 0x00
};


ArduCAMStream::ArduCAMStream(ArduCAM *myCam) {
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
		if (myCAM->get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {

			len = myCAM->read_fifo_length();
//			if (myCAM.get_format() == BMP)
//				len += BMPIMAGEOFFSET;
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
	if(!dataReady()) {
		return 0;
	}

	if (!transfer) {
		transfer = true;

		myCAM->CS_LOW();
		myCAM->set_fifo_burst();
		// clear dummy byte -> send out LO and ignore MISO
		SPI.transfer(0x0);

		// send a BMP header for BMP files
		if (myCAM->get_format() == BMP) {
			hdump.print((unsigned char *)bmp_header, BMPIMAGEOFFSET);
			memcpy(data, (unsigned char *)bmp_header, BMPIMAGEOFFSET);
			return BMPIMAGEOFFSET;
		}

	}

	int bytesread = min(len, bufSize);

	ACAM_DEBUG("ArduCAMStream::readMemoryBlock [%d] (%d bytes) remaining (%d bytes)\n", bcount++, bytesread, len);

	// block read does overload the cam
//	HwSPI.recv((uint8_t *)data, bytesread);
//	SPI.transfer((unsigned char*)data, bytesread);

	// so we have to read single bytes
	for (int i=0; i< bytesread; i++) {
		data[i] = SPI.read8();
	}
	len = len - bytesread;

	if (len == 0) {
		ACAM_DEBUG("ArduCAMStream::readMemoryBlock -> eof");
		transfer = false;
		SPI.endTransaction();
		myCAM->CS_HIGH();
	}

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


