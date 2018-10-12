/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpResponse
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_RESPONSE_H_
#define _SMING_CORE_HTTP_RESPONSE_H_

#include "HttpCommon.h"
#include "Data/Stream/TemplateFileStream.h"
#include "Network/Http/HttpHeaders.h"

class JsonObjectStream; // << TODO: deprecated and should be removed in the next version

class HttpResponse
{
	friend class HttpClient;
	friend class HttpConnection;
	friend class HttpServerConnection;

public:
	~HttpResponse();

	bool sendString(const String& text);

	// @deprecated method

	bool hasHeader(const String& name);

	void redirect(const String& location);

	/**
	 * @deprecated Use response.code = HTTP_STATUS_FORBIDDEN instead
	 */
	__forceinline void forbidden()
	{
		code = HTTP_STATUS_FORBIDDEN;
	}

	/**
	 * @deprecated Use response.code = HTTP_STATUS_NOT_FOUND instead
	 */
	__forceinline void notFound()
	{
		code = HTTP_STATUS_NOT_FOUND;
	}

	HttpResponse* setContentType(const String& type);
	HttpResponse* setContentType(enum MimeType type);
	HttpResponse* setCookie(const String& name, const String& value);
	HttpResponse* setHeader(const String& name, const String& value);
	HttpResponse* setCache(int maxAgeSeconds = 3600, bool isPublic = false);
	HttpResponse* setAllowCrossDomainOrigin(
		const String& controlAllowOrigin); // Access-Control-Allow-Origin for AJAX from a different domain

	// Send file by name
	bool sendFile(String fileName, bool allowGzipFileCheck = true);

	// @deprecated

	// Parse and send template file
	bool sendTemplate(TemplateFileStream* newTemplateInstance);

	/**
	 * @brief Build and send JSON string
	 *
	 * @deprecated use response.sendDataStream(stream, MIME_JSON) instead
	 */
	bool sendJsonObject(JsonObjectStream* newJsonStreamInstance);

	// @end deprecated

	// Send Datastream, can be called with Classes derived from
	bool sendDataStream(ReadWriteStream* newDataStream, enum MimeType type)
	{
		return sendDataStream(newDataStream, ContentType::toString(type));
	}

	// Send Datastream, can be called with Classes derived from
	bool sendDataStream(ReadWriteStream* newDataStream, const String& reqContentType = nullptr);

	String getBody();

	void reset();

public:
	int code;
	HttpHeaders headers;
	ReadWriteStream* stream = nullptr;
};

#endif /* _SMING_CORE_HTTP_RESPONSE_H_ */
