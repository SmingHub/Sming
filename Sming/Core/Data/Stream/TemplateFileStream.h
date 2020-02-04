/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * TemplateFileStream.h
 *
 ****/

#pragma once

#include "FileStream.h"
#include "TemplateStream.h"

/**
  * @brief Template stream using content from the filesystem
  * @ingroup stream data
 */
class TemplateFileStream : public TemplateStream
{
public:
	TemplateFileStream(const String& fileName) : TemplateStream(new FileStream(fileName))
	{
	}
};
