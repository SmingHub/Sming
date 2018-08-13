/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/*
 * WebConstants.cpp
 *
 *  Created on: 26 Jul 2018
 *      Author: mikee47
 *
 */

#include "WebConstants.h"
#include "FakePgmSpace.h"

namespace ContentType {
String fromFileExtension(const char* extension)
{
	// We accept 'htm' or 'html'
	if (strcasecmp(extension, _F("htm")) == 0)
		return toString(MIME_HTML);

#define XX(_name, _ext, _mime)                                                                                         \
	if (strcasecmp(extension, _F(_ext)) == 0)                                                                          \
		return F(_mime);
	MIME_TYPE_MAP(XX)
#undef XX

	// Type undefined - if (String) will return false
	return nullptr;
}

String fromFileExtension(const String& extension)
{
	return fromFileExtension(extension.c_str());
}

String toString(enum MimeType m)
{
#define XX(name, extensionStart, mime)                                                                                 \
	if (MIME_##name == m)                                                                                              \
		return F(mime);
	MIME_TYPE_MAP(XX)
#undef XX

	return nullptr;
}

String fromFileName(const String& fileName)
{
	int p = fileName.lastIndexOf('.');
	if (p < 0)
		return nullptr;

	const char* ext = fileName.c_str() + p + 1;
	return fromFileExtension(ext);
}
}; // namespace ContentType
