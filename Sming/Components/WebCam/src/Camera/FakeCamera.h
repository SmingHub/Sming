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

class FakeCamera: public CameraInterface
{
public:

	/**
	 * Sets the list of all images that should be used to roll over.
	 */
	FakeCamera(const Vector<String>& files)
	{
		this->files = files;
	}

	~FakeCamera()
	{
		delete file;
		file = nullptr;
	}

	const String getMimeType() const override
	{
		return "image/jpeg";
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

		state = eWCS_INITIALISING;
		spiffs_mount(); // Mount file system, in order to work with files
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

		// go to the next picture.
		delete file;
		file = new FileStream(files[index++]);
		if(index == files.count()) {
			index = 0;
		}
		if(!file->isValid()) {
			delete file;
			file = nullptr;
			return false;
		}
		state = eWCS_HAS_PICTURE;

		return true;
	}

	/**
	 * Gets the size of the current picture
	 */
	size_t getSize() override
	{
		if(file == nullptr) {
			return 0;
		}

		return file->getSize();
	}

	/**
	 * @brief Read picture data from the camera.
	 * @param buffer the allocated data buffer to store the data
	 * @param size the size of the allocated buffer
	 * @param offset
	 *
	 * @retval bytes successfully read and stored in the buffer
	 */
	size_t read(char* buffer, size_t size, size_t offset = 0)
	{
		if(file == nullptr) {
			return 0;
		}

		// get the current picture and read the desired data from it.
		file->seekFrom(offset, SEEK_SET);
		return file->readMemoryBlock(buffer, size);
	}

private:
	size_t index = 0;
	FileStream* file = nullptr;
	Vector<String> files;
};
