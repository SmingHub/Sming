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
#include <FakePgmSpace.h>
#include <Data/CStringArray.h>
#include <stringutil.h>

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

MimeType fromFileExtension(const char* extension, MimeType unknown)
{
	// We accept 'htm' or 'html', but the latter is preferred
	if(strcasecmp(extension, _F("htm")) == 0) {
		return MIME_HTML;
	}

	int i = CStringArray(fstr_ext).indexOf(extension);
	return (i < 0) ? unknown : MimeType(i);
}

String fromFileExtension(const char* extension)
{
	auto mime = fromFileExtension(extension, MIME_UNKNOWN);
	if(mime == MIME_UNKNOWN) {
		// Type undefined - if (String) will return false
		return nullptr;
	}

	return toString(mime);
}

String toString(MimeType m)
{
	if(m == MIME_UNKNOWN) {
		return nullptr;
	}

	return CStringArray(fstr_mime)[m];
}

MimeType fromString(const char* str)
{
	int i = CStringArray(fstr_mime).indexOf(str);
	if(i < 0) {
		if(strcasecmp(str, _F("application/xml")) == 0) {
			i = MIME_XML;
		}
	}
	return (i < 0) ? MIME_UNKNOWN : MimeType(i);
}

MimeType fromFullFileName(const char* fileName, MimeType unknown)
{
	if(fileName == nullptr) {
		return unknown;
	}

	const char* extension = strrchr(fileName, '.');
	return extension ? fromFileExtension(extension + 1, unknown) : unknown;
}

String fromFullFileName(const char* fileName)
{
	MimeType mime = fromFullFileName(fileName, MIME_UNKNOWN);
	return toString(mime);
}

}; // namespace ContentType
