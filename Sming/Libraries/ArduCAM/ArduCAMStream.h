/*
 * ArduCAMStream.h
 *
 *  Created on: Feb 13, 2016
 *      Author: harry
 */

#ifndef SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_
#define SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_

#include "ArduCAM.h"

#include "../../Services/HexDump/HexDump.h"

class ArduCAMStream : public ReadWriteStream {
public:
	ArduCAMStream(ArduCAM* cam);
	virtual ~ArduCAMStream();

	virtual StreamType getStreamType() const
	{
		return eSST_User;
	}

	virtual size_t readMemoryBlock(char* data, size_t bufSize);
	virtual bool seek(int len);
	virtual bool isFinished();

	virtual size_t write(uint8_t charToWrite)
	{
		return 0;
	}

	/** @brief  Write chars to stream
	*  @param  buffer Pointer to buffer to write to the stream
	*  @param  size Quantity of chars to writen
	*  @retval size_t Quantity of chars written to stream
	*/
	virtual size_t write(const uint8_t* buffer, size_t size)
	{
		return 0;
	}

	bool dataReady();
	int available();

private:
	ArduCAM* myCAM;
	bool transfer = false;
	bool sendHeader = false;
	size_t len;
	int bcount;
	HexDump hdump;
};

#endif /* SAMPLES_ARDUCAM_APP_ARDUCAMSTREAM_H_ */
