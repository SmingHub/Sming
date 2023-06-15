/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * DirectoryTemplate.h
 *
 * @author mikee47 <mike@sillyhouse.net> May 2019
 *
 *
 ****/

#pragma once

#include "../SectionTemplate.h"
#include <IFS/Directory.h>

#define DIRSTREAM_FIELD_MAP(XX)                                                                                        \
	XX(file_id, "File identifier")                                                                                     \
	XX(name, "Filename")                                                                                               \
	XX(modified, "Date/time of file modification")                                                                     \
	XX(size, "File size in bytes")                                                                                     \
	XX(original_size, "Original size of compressed file")                                                              \
	XX(attr, "File attributes (brief)")                                                                                \
	XX(attr_long, "File attributes")                                                                                   \
	XX(compression, "Compression type")                                                                                \
	XX(access, "File access information (brief)")                                                                      \
	XX(access_long, "File access information")                                                                         \
	XX(index, "Zero-based index of current file")                                                                      \
	XX(total_size, "Total size of files processed (in bytes)")                                                         \
	XX(path, "Path to containing directory")                                                                           \
	XX(parent, "Path to parent directory (if any)")                                                                    \
	XX(last_error, "Last error message")

namespace IFS
{
/**
  * @brief      Directory stream class
  * @ingroup    stream data
 */
class DirectoryTemplate : public SectionTemplate
{
public:
	enum class Field {
		unknown = 0,
#define XX(name, comment) name,
		DIRSTREAM_FIELD_MAP(XX)
#undef XX
	};

	DirectoryTemplate(IDataSourceStream* source, Directory* dir) : SectionTemplate(source)
	{
		directory.reset(dir);
	}

	Directory& dir()
	{
		return *directory;
	}

	bool nextRecord() override
	{
		if(sectionIndex() == 1) {
			return directory->next();
		}

		return recordIndex() < 0;
	}

protected:
	String getValue(const char* name) override;

private:
	std::unique_ptr<Directory> directory;
};

} // namespace IFS
