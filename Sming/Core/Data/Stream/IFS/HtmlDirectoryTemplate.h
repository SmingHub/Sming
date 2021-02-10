/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HtmlDirectoryTemplate.h
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 ****/

#pragma once

#include "../../Format.h"
#include "DirectoryTemplate.h"

namespace IFS
{
/**
 * @brief Read-only stream access to directory listing with HTML output
 * @ingroup stream data
*/
class HtmlDirectoryTemplate : public DirectoryTemplate
{
public:
	HtmlDirectoryTemplate(IDataSourceStream* source, Directory* dir) : DirectoryTemplate(source, dir)
	{
		setFormatter(Format::html);
	}

protected:
	String getValue(const char* name) override;
};

} // namespace IFS
