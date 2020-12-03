/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HostFileStream.cpp
 *
 ****/

#include "HostFileStream.h"
#include <IFS/Host/FileSystem.h>

namespace
{
IFS::Host::FileSystem hostFileSystem;
}

HostFileStream::HostFileStream() : IFS::FileStream(&hostFileSystem)
{
}
