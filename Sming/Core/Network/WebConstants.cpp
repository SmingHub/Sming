/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WebConstants.cpp
 *
 ****/

#include "WebConstants.h"
#include "FakePgmSpace.h"

namespace ContentType
{
// MIME type strings
#define XX(_name, _ext, _mime) DEFINE_FSTR_LOCAL(mimestr_##_name, _mime);
MIME_TYPE_MAP(XX)
#undef XX

static FSTR_TABLE(mimeStrings) = {
#define XX(_name, _ext, _mime) FSTR_PTR(mimestr_##_name),
	MIME_TYPE_MAP(XX)
#undef XX
};

// File extensions
#define XX(_name, _ext, _mime) DEFINE_FSTR_LOCAL(extstr_##_name, _ext);
MIME_TYPE_MAP(XX)
#undef XX

static FSTR_TABLE(extensionStrings) = {
#define XX(_name, _ext, _mime) FSTR_PTR(extstr_##_name),
	MIME_TYPE_MAP(XX)
#undef XX
};

String fromFileExtension(const char* extension)
{
	// We accept 'htm' or 'html', but the latter is preferred
	if(strcasecmp(extension, _F("htm")) == 0)
		return mimestr_HTML;

	for(unsigned i = 0; i < ARRAY_SIZE(extensionStrings); ++i) {
		if(*extensionStrings[i] == extension)
			return *mimeStrings[i];
	}

	// Type undefined - if (String) will return false
	return nullptr;
}

String toString(enum MimeType m)
{
	if(m >= ARRAY_SIZE(mimeStrings))
		return nullptr;

	return *mimeStrings[m];
}

String fromFullFileName(const char* fileName)
{
	if(!fileName)
		return nullptr;

	const char* extension = strrchr(fileName, '.');

	return extension ? fromFileExtension(extension + 1) : nullptr;
}

}; // namespace ContentType
