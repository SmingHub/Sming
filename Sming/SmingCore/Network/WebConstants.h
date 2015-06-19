/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_WEBCONSTANTS_H_
#define _SMING_CORE_NETWORK_WEBCONSTANTS_H_

namespace ContentType
{
	// Texts
	static const char* HTML = "text/html";
	static const char* TEXT = "text/plain";
	static const char* JS = "text/javascript";
	static const char* CSS = "text/css";
	static const char* XML = "text/xml";

	// Images
	static const char* JPEG = "image/jpeg";
	static const char* GIF = "image/gif";
	static const char* PNG = "image/png";
	static const char* SVG = "image/svg+xml";
	static const char* ICO = "image/x-icon";

	static const char* GZIP = "application/x-gzip";
	static const char* ZIP = "application/zip";
	static const char* JSON = "application/json";

	static const char* Binary = "application/octet-stream";
	static const char* FormUrlEncoded = "application/x-www-form-urlencoded";
	static const char* FormMultipart = "multipart/form-data";

	static const char* fromFileExtension(const String extension)
	{
		String ext = extension;
		ext.toLowerCase();

		if (ext.startsWith("htm")) return HTML;
		if (ext.equals("txt")) return TEXT;
		if (ext.equals("js")) return JS;
		if (ext.equals("css")) return CSS;

		if (ext.equals("jpg") | ext.equals("jpeg")) return TEXT;
		if (ext.equals("gif")) return GIF;
		if (ext.equals("png")) return PNG;
		if (ext.equals("svg")) return SVG;
		if (ext.equals("zip")) return ZIP;

		// Not found, we can't be sure
		return NULL;
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

namespace RequestMethod
{
	static const char* GET = "GET";
	static const char* HEAD = "HEAD";
	static const char* POST = "POST";
	static const char* PUT = "PUT";
	static const char* DELETE = "DELETE";
};

namespace HttpStatusCode
{
	static const char* OK = "200 OK";
	static const char* SwitchingProtocols = "101 Switching Protocols";
	static const char* Found = "302 Found";

	static const char* BadRequest = "400 Bad Request";
	static const char* NotFound = "404 Not Found";
	static const char* Forbidden = "403 Forbidden";
	static const char* Unauthorized = "401 Unauthorized";

	static const char* NotImplemented = "501 Not Implemented";
};

#endif /* _SMING_CORE_NETWORK_WEBCONSTANTS_H_ */
