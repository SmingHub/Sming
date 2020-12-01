/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * GdbFileStream.h
 *
 ****/

#pragma once

#include "IFS/FileStream.h"
#include <IFS/Gdb/FileSystem.h>

/**
  * @brief GDB File stream class to provide access to host files whilst running under debugger
  * @ingroup stream gdb_syscall
 */
class GdbFileStream : public IFS::FileStream
{
public:
	GdbFileStream() : IFS::FileStream(&fileSystem)
	{
	}

private:
	IFS::Gdb::FileSystem fileSystem;
};
