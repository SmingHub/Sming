/*
 * CamSettings.h
 *
 *  Created on: Feb 4, 2016
 *      Author: harry
 */

#ifndef SAMPLES_ARDUCAM_APP_CAMSETTINGS_H_
#define SAMPLES_ARDUCAM_APP_CAMSETTINGS_H_

#define  AC_IMAGE_BMP  1
#define  AC_IMAGE_JPEG 2

#define  AC_IMAGE_160 	1
#define  AC_IMAGE_176 	2

#include "../include/user_config.h"
#include <SmingCore/SmingCore.h>

#include <Libraries/ArduCAM/ov2640_regs.h>


class CamSettings {
public:
	CamSettings();
	virtual ~CamSettings();

	const char * getImageType();
	String getContentType();
	void setImageType(int type);
	byte getImageTypeCode();

	const char * getImageSize();
	void setImageSize(int size);
	uint8_t getImageSizeCode();

private:
	int _ImageType = BMP;
	int _ImageSize = OV2640_160x120;
};

#endif /* SAMPLES_ARDUCAM_APP_CAMSETTINGS_H_ */
