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
#include "WebcamPictureStream.h"

/**
 * @brief Webcam stream producer for multipart streaming
 */
class WebcamStream : public MultipartStream
{
public:
	WebcamStream(CameraInterface& camera) : MultipartStream(std::bind(&WebcamStream::produce, this)), camera(camera)
	{
	}

	uint16_t readMemoryBlock(char* data, int bufSize) override
	{
		if(camera.getState() == eWCS_READY) {
			uint8_t fps = camera.getFramesPerSecond();
			if(fps > 0 && lastFrameTime) {
				uint16_t frameTimeMs = 1000 / fps;
				uint32_t now = millis();
				if(now < lastFrameTime + frameTimeMs) {
					debug_d("Skipping frame to maintain fps");
					return 0;
				}
			}

			camera.capture();
			lastFrameTime = millis();
		}

		return MultipartStream::readMemoryBlock(data, bufSize);
	}

	MultipartStream::BodyPart produce()
	{
		MultipartStream::BodyPart result;

		camera.capture();
		lastFrameTime = millis();

		WebcamPictureStream* webcamStream = new WebcamPictureStream(camera);
		result.stream = webcamStream;

		result.headers = new HttpHeaders();
		(*result.headers)[HTTP_HEADER_CONTENT_TYPE] = camera.getMimeType();
		(*result.headers)[HTTP_HEADER_CONTENT_LENGTH] = camera.getSize();

		return result;
	}

private:
	CameraInterface& camera;
	unsigned long lastFrameTime = 0;
};
