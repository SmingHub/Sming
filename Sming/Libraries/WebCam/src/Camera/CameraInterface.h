/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Camera/CameraInterface.h
 *
 * @author: 2019 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once
#include <stdint.h>

enum CameraState {
	eWCS_NOT_READY,
	eWCS_INITIALISING,
	eWCS_READY,
	eWCS_WORKING,
	eWCS_HAS_PICTURE,
};

class CameraInterface
{
public:
	virtual ~CameraInterface()
	{
	}

	virtual const String getMimeType() const = 0;

	/**
	 * @brief Initializes the camera
	 *
	 * @retval true on success. The camera state should also change to eWCS_READY
	 */
	virtual bool init() = 0;

	/**
	 * @brief Gets the current state of the camera
	 * @retval CameraState
	 */
	CameraState getState() const
	{
		return state;
	}

	/**
	 * Prepare for the next picture
	 */
	virtual void next()
	{
		if(state == eWCS_HAS_PICTURE) {
			state = eWCS_READY;
		}
	}

	/**
	 * @brief Instructs the camera to capture new picture.
	 */
	virtual bool capture() = 0;

	/**
	 * Gets the size of the current picture
	 */
	virtual size_t getSize() = 0;

	/**
	 * @brief Read picture data from the camera.
	 * @param buffer the allocated data buffer to store the data
	 * @param size the size of the allocated buffer
	 * @param offset
	 *
	 * @retval bytes successfully read and stored in the buffer
	 */
	virtual size_t read(char* buffer, size_t size, size_t offset = 0) = 0;

protected:
	CameraState state = eWCS_NOT_READY;
};
