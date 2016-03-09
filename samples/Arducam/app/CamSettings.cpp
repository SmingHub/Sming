/*
 * CamSettings.cpp
 *
 *  Created on: Feb 4, 2016
 *      Author: harry
 */

#include "CamSettings.h"


CamSettings::CamSettings() {
	// TODO Auto-generated constructor stub

}

CamSettings::~CamSettings() {
	// TODO Auto-generated destructor stub
}


const char * CamSettings::getImageType() {
	switch (_ImageType) {
	case JPEG:
		return "JPEG";
	case BMP:
	default:
		return "BMP";
	}
}

String CamSettings::getContentType() {
	switch (_ImageType) {
	case JPEG:
		return "image/jpeg";
	case BMP:
	default:
		return "image/x-ms-bmp";
	}
}

byte CamSettings::getImageTypeCode() {
	return (byte) _ImageType;
}

void CamSettings::setImageType(int type) {
	_ImageType = type;
}

const char * CamSettings::getImageSize() {
	switch (_ImageSize) {
	case OV2640_1600x1200:
		return  "1600x1200";
	case OV2640_1280x1024:
		return  "1280x1024";
	case OV2640_1024x768:
		return  "1024x768";
	case OV2640_800x600:
		return  "800x600";
	case OV2640_640x480:
		return  "640x480";
	case OV2640_352x288:
		return  "352x288";
	case OV2640_320x240:
		return  "320x240";
	case OV2640_176x144:
		return "176x144";
	case OV2640_160x120:
	default:
		return "160x120";
	}
}

uint8_t CamSettings::getImageSizeCode() {
	return _ImageSize;
}


void CamSettings::setImageSize(int size) {
	// todo validate input !!
	_ImageSize = size;
}
