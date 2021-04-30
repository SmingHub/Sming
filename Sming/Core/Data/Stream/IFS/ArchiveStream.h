/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * ArchiveStream.h
 *
 * @author mikee47 <mike@sillyhouse.net> April 2021
 *
 *
 ****/

#pragma once

#include <IFS/FWFS/ArchiveStream.h>
#include <FileSystem.h>

class ArchiveStream : public IFS::FWFS::ArchiveStream
{
public:
	using IFS::FWFS::ArchiveStream::ArchiveStream;

	ArchiveStream(VolumeInfo volumeInfo, String rootPath = nullptr, Flags flags = 0)
		: ArchiveStream(::getFileSystem(), volumeInfo, rootPath, flags)
	{
	}
};
