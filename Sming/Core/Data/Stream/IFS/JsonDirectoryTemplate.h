/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * JsonDirectoryTemplate.h
 *
 * @author mikee47 <mike@sillyhouse.net> Nov 2020
 *
 ****/

#pragma once

#include "DirectoryTemplate.h"
#include <Data/Format.h>

namespace IFS
{
/**
 * @brief Read-only stream providing directory listing in JSON format
*/
class JsonDirectoryTemplate : public DirectoryTemplate
{
public:
	JsonDirectoryTemplate(IDataSourceStream* source, Directory* dir) : DirectoryTemplate(source, dir)
	{
		setFormatter(Format::json);
	}
};

} // namespace IFS
