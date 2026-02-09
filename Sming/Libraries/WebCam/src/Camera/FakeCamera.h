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
		auto& filename = files[index++];
		if(index == files.count()) {
			index = 0;
		}
		if(!file.open(filename)) {
			return false;
		}
		state = eWCS_HAS_PICTURE;

		return true;
	}

	/**
	 * @brief Gets the size of the current picture
	 */
	size_t getSize() override
	{
		return file.getSize();
	}

	uint8_t getFramesPerSecond() override
	{
		return 10; // fake camera supports 10 fps
	}

	/**
	 * @brief Read picture data from the camera.
	 * @param buffer the allocated data buffer to store the data
	 * @param size the size of the allocated buffer
	 * @param offset
	 *
	 * @retval bytes successfully read and stored in the buffer
	 */
	size_t read(char* buffer, size_t size, size_t offset = 0) override
	{
		// get the current picture and read the desired data from it.
		file.seekFrom(offset, SeekOrigin::Start);
		return file.readMemoryBlock(buffer, size);
	}

private:
	size_t index{0};
	FileStream file;
	Vector<String> files;
};
