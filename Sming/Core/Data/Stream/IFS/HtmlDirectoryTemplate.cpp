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

#include "HtmlDirectoryTemplate.h"
#include <Network/WebConstants.h>
#include <Network/WebHelpers/escape.h>

namespace IFS
{
String HtmlDirectoryTemplate::getValue(const char* name)
{
	String text = DirectoryTemplate::getValue(name);
	if(text) {
		return text;
	}

	if(dir().isValid()) {
		auto& stat = dir().stat();

		if(FS("icon") == name) {
			if(stat.attr[FileAttribute::Directory]) {
				return F("&#128193;");
			}

			auto mimeType = ContentType::fromFullFileName(stat.name, MIME_UNKNOWN);
			// https://html-css-js.com/html/character-codes/icons/
			switch(mimeType) {
			case MIME_TEXT:
				return F("&#128196;");
			case MIME_JS:
			case MIME_CSS:
			case MIME_HTML:
			case MIME_XML:
			case MIME_JSON:
				return F("&#128209;");

			case MIME_JPEG:
			case MIME_GIF:
			case MIME_PNG:
			case MIME_SVG:
			case MIME_ICO:
				return F("&#127827;");

			case MIME_GZIP:
			case MIME_ZIP:
				return F("&#128451;");

			case MIME_UNKNOWN:
			default:
				return F("&bull;");
			}
		}
	}

	return nullptr;
}

} // namespace IFS
