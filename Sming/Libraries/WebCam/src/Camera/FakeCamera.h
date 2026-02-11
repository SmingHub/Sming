/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Camera/FakeCamera.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "CameraInterface.h"
#include <Data/Stream/FileStream.h>

class FakeCamera : public CameraInterface
{
public:
	/**
	 * @brief Sets the list of all images that should be used to roll over.
	 */
	FakeCamera()
	{
	}

	void addImage(const String& filename)
	{
		files.add(filename);
	}

	const String getMimeType() const override
	{
		return toString(MIME_JPEG);
	}

	/**
	 * @brief Initializes the camera
	 *
	 * @retval true on success. The camera state should also change to eWCS_READY
	 */
	bool init() override
	{
		if(state == eWCS_INITIALISING) {
			return true;
		}

		state = eWCS_READY;
		index = 0;
		return true;
	}

	/**
	 * @brief Instructs the camera to capture new picture.
	 */
	bool capture() override
	{
		if(state == eWCS_NOT_READY && !init()) {
			return false;
		}

		if(files.count() == 0) {
			debug_w("No images added to the fake camera");
			return false;
		}

		// go to the next picture.
		index++;
		if(index == files.count()) {
			index = 0;
		}

		state = eWCS_HAS_PICTURE;

		return true;
	}

	uint8_t getFramesPerSecond() override
	{
		return 1; // fake camera supports 10 fps
	}

	IDataSourceStream* newImageStream() override
	{
		return new FileStream(files[index]);
	}

private:
	size_t index{0};
	FileStream file;
	Vector<String> files;
};
