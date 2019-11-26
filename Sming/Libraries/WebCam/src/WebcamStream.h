/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebcamStream.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include <Data/Stream/DataSourceStream.h>
#include "Camera/CameraInterface.h"

class WebcamStream: public IDataSourceStream
{
public:
	/**
	 * @param camera pointer to the camera object.
	 */
	WebcamStream(CameraInterface* camera, size_t blockSize = 512): camera(camera), blockSize(blockSize)
	{
		assert(camera != nullptr);
	}

	uint16_t readMemoryBlock(char* data, int bufSize)
	{
		if(camera->getState() == eWCS_HAS_PICTURE) {
			if(!size) {
				size = camera->getSize();
				offset = 0;
			}

			return camera->read(data, bufSize, offset);
		}

		if(camera->getState() != eWCS_WORKING) {
			if(camera->capture()) {
				size = 0;
				offset = 0;
			}
		}

		return 0;
	}

	int available()
	{
		return size;
	}

	bool seek(int len)
	{
		offset += len; // TODO: check for invalid offsets
		return true;
	}

	bool isFinished()
	{
		bool finished = (size && offset >= size);
		if(finished) {
			camera->next();
		}
		return finished;
	}

	~WebcamStream()
	{
		camera = nullptr;
	}
private:
	CameraInterface* camera = nullptr;
	size_t blockSize;
	size_t size = 0;
	size_t offset = 0;
};
