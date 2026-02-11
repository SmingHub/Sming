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

		result.stream = camera.newImageStream();

		result.headers = new HttpHeaders();
		(*result.headers)[HTTP_HEADER_CONTENT_TYPE] = camera.getMimeType();
		(*result.headers)[HTTP_HEADER_CONTENT_LENGTH] = result.stream->available();

		return result;
	}

private:
	CameraInterface& camera;
	unsigned long lastFrameTime = 0;
};
