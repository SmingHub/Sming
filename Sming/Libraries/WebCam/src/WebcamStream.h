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

#include <Data/Stream/MultipartStream.h>
#include "Camera/CameraInterface.h"
#include <Data/Range.h>

/**
 * @brief Webcam stream producer for multipart streaming
 */
class WebcamStream : public MultipartStream
{
public:
	WebcamStream(CameraInterface& camera) : MultipartStream(std::bind(&WebcamStream::produce, this)), camera(camera)
	{
	}

	MultipartStream::BodyPart produce()
	{
		MultipartStream::BodyPart result;

		camera.capture();
		result.stream = camera.newImageStream();

		result.headers = new HttpHeaders();
		(*result.headers)[HTTP_HEADER_CONTENT_TYPE] = camera.getMimeType();
		(*result.headers)[HTTP_HEADER_CONTENT_LENGTH] = result.stream->available();

		// Don't count first image
		if(started) {
			newImage = true;
		} else {
			// Note: This won't take into account changes of camera frame rate
			// int fps = TRange(1, 100).clip(camera.getFramesPerSecond());
			// frameTimer.reset(1000 / fps);
			frameTimer.reset(250);
			started = true;
		}

		return result;
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		if(newImage) {
			if(!frameTimer.expired()) {
				debug_d("Defer read to maintain fps");
				return 0;
			}
			newImage = false;
			frameTimer.start();
		}

		return MultipartStream::readMemoryBlock(data, bufSize);
	}

private:
	CameraInterface& camera;
	OneShotFastMs frameTimer;
	bool started{false};
	bool newImage{false};
};
