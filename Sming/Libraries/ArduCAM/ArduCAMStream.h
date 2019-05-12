/*
 * ArduCAMStream.h
 *
 *  Created on: Feb 13, 2016
 *      Author: harry
 */

#ifndef SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_
#define SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_

#include "ArduCAM.h"

#include <Services/HexDump/HexDump.h>


class ArduCAMStream: public IDataSourceStream {
public:
	ArduCAMStream(ArduCAM *cam);
	virtual ~ArduCAMStream();

	virtual StreamType getStreamType() const { return eSST_User; }

	virtual uint16_t readMemoryBlock(char* data, int bufSize);
	virtual bool seek(int len);
	virtual bool isFinished();

	bool dataReady();
	int available();

private:

	ArduCAM *myCAM;
	bool transfer = false;
	bool sendHeader = false;
	size_t len;
	int bcount;
	HexDump hdump;
};

#endif /* SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_ */
