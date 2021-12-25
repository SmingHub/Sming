/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HostFileStream.h
 *
 ****/

#pragma once

#include <Data/Stream/IFS/FileStream.h>
#include <IFS/File.h>

/**
  * @brief      Host File stream class
  * @ingroup    stream data
 */
class HostFileStream : public IFS::FileStream
{
public:
	HostFileStream();

	HostFileStream(const String& fileName, IFS::OpenFlags openFlags = IFS::OpenFlag::Read) : HostFileStream()
	{
		open(fileName, openFlags);
	}
};
