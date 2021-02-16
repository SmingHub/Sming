/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Directory.h
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#pragma once

#include <IFS/Directory.h>
#include <FileSystem.h>

/**
  * @brief      Directory stream class
  * @ingroup    stream data
 */
class Directory : public IFS::Directory
{
public:
	Directory() : IFS::Directory(::getFileSystem())
	{
	}
};
