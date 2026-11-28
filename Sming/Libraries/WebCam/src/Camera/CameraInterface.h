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
	virtual ~CameraInterface() = default;

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
	 * @brief Prepare for the next picture
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
	 * @brief Gets the frames per second the camera can capture
	 */
	virtual uint8_t getFramesPerSecond() = 0;

	/** @brief Create a new image stream. The stream is owned by the caller and must be deleted.
	 *  @retval IDataSourceStream Pointer to the new image stream
	 */
	virtual IDataSourceStream* newImageStream() = 0;

protected:
	CameraState state = eWCS_NOT_READY;
};
