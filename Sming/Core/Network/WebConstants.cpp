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
#include <Data/CStringArray.h>

namespace ContentType
{
// MIME type strings
#define XX(name, ext, mime) mime "\0"
DEFINE_FSTR_LOCAL(fstr_mime, MIME_TYPE_MAP(XX))
#undef XX

// File extensions
#define XX(name, ext, mime) ext "\0"
DEFINE_FSTR_LOCAL(fstr_ext, MIME_TYPE_MAP(XX))
#undef XX

String fromFileExtension(const char* extension)
{
	CStringArray mimeStrings(fstr_mime);

	// We accept 'htm' or 'html', but the latter is preferred
	if(strcasecmp(extension, _F("htm")) == 0) {
		return mimeStrings[MIME_HTML];
	}

	int i = CStringArray(fstr_ext).indexOf(extension);
	if(i < 0) {
		// Type undefined - if (String) will return false
		return nullptr;
	}

	return mimeStrings[i];
}

String toString(enum MimeType m)
{
	return CStringArray(fstr_mime)[m];
}

MimeType fromString(const char* str)
{
	int i = CStringArray(fstr_mime).indexOf(str);
	return (i < 0) ? MIME_UNKNOWN : MimeType(i);
}

String fromFullFileName(const char* fileName)
{
	if(fileName == nullptr) {
		return nullptr;
	}

	const char* extension = strrchr(fileName, '.');

	return extension ? fromFileExtension(extension + 1) : nullptr;
}

}; // namespace ContentType
