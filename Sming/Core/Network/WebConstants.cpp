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
#include <FlashString/Vector.hpp>
#include <stringutil.h>

namespace
{
// MIME type strings
#define XX(name, ext, mime) DEFINE_FSTR_LOCAL(str_contenttype_##name, mime)
MIME_TYPE_MAP(XX)
#undef XX

#define XX(name, ext, mime) &str_contenttype_##name,
DEFINE_FSTR_VECTOR(contentTypeStrings, FlashString, MIME_TYPE_MAP(XX))
#undef XX

// File extensions
#define XX(name, ext, mime) DEFINE_FSTR_LOCAL(str_ext_##name, ext)
MIME_TYPE_MAP(XX)
#undef XX

#define XX(name, ext, mime) &str_ext_##name,
DEFINE_FSTR_VECTOR(extensionStrings, FlashString, MIME_TYPE_MAP(XX))
#undef XX
} // namespace

String toString(MimeType m)
{
	if(m == MIME_UNKNOWN) {
		return nullptr;
	}

	return contentTypeStrings[unsigned(m)];
}

namespace ContentType
{
MimeType fromFileExtension(const char* extension, MimeType unknown)
{
	// We accept 'htm' or 'html', but the latter is preferred
	if(strcasecmp(extension, _F("htm")) == 0) {
		return MIME_HTML;
	}

	int i = extensionStrings.indexOf(extension);
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

MimeType fromString(const char* str)
{
	int i = contentTypeStrings.indexOf(str);
	if(i < 0) {
		if(strcasecmp(str, _F("application/xml")) == 0) {
			return MIME_XML;
		} else {
			return MIME_UNKNOWN;
		}
	}
	return MimeType(i);
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
