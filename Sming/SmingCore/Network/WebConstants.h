/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_WEBCONSTANTS_H_
#define _SMING_CORE_NETWORK_WEBCONSTANTS_H_

#define MIME_TYPE_MAP(XX)                  \
  /* Type, extension start, Mime type */   \
							               \
  /* Texts */				               \
  XX(HTML, "htm", "text/html")             \
  XX(TEXT, "txt", "text/plain")            \
  XX(JS, "js", "text/javascript")          \
  XX(CSS, "css", "text/css")               \
  XX(XML, "xml", "text/xml")               \
  XX(JSON, "json", "application/json")     \
  	  	  	  	  	  	  	               \
  /* Images */                             \
  XX(JPEG, "jpg", "image/jpeg")            \
  XX(GIF, "git", "image/gif")              \
  XX(PNG, "png", "image/png")              \
  XX(SVG, "svg", "image/svg+xml")          \
  XX(ICO, "ico", "image/x-icon")           \
                                           \
  /* Archives */                           \
  XX(GZIP, "gzip", "application/x-gzip")   \
  XX(ZIP, "zip", "application/zip")        \
  	  	  	  	  	  	  	  	           \
  /* Binary and Form */                    \
  XX(BINARY, "", "application/octet-stream")   \
  XX(FORM_URL_ENCODED, "", "application/x-www-form-urlencoded") \
  XX(FORM_MULTIPART, "", "multipart/form-data") \

enum MimeType
{
#define XX(name, extensionStart, mime) MIME_##name,
	MIME_TYPE_MAP(XX)
#undef XX
};

namespace ContentType
{
	static const char* fromFileExtension(const String extension)
	{
		String ext = extension;
		ext.toLowerCase();

		#define XX(name, extensionStart, mime) if(ext.startsWith(extensionStart)) {  return mime; }
		  MIME_TYPE_MAP(XX)
		#undef XX

		// Unknown
		return "<unknown>";
	}

	static const char *toString(enum MimeType m)
	{
		#define XX(name, extensionStart, mime) if(MIME_##name == m) {  return mime; }
		  MIME_TYPE_MAP(XX)
		#undef XX

		// Unknown
		return "<unknown>";
	}

	static const char* fromFullFileName(const String fileName)
	{
		int p = fileName.lastIndexOf('.');
		if (p != -1)
		{
			String ext = fileName.substring(p + 1);
			const char *mime = ContentType::fromFileExtension(ext);
			return mime;
		}

		return NULL;
	}
};

#endif /* _SMING_CORE_NETWORK_WEBCONSTANTS_H_ */
