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
#include "Data/Stream/ReadWriteStream.h"
#include "HttpHeaders.h"
#include "FileSystem.h"

class JsonObjectStream; // << TODO: deprecated and should be removed in the next version
class TemplateStream;

class HttpResponse
{
public:
	~HttpResponse()
	{
		freeStreams();
	}

	bool sendString(const String& text);

	/**
	 * @deprecated use headers.contains() method
	 */
	bool hasHeader(const String& name) __deprecated
	{
		return headers.contains(name);
	}

	/**
	 * @deprecated use headers[HTTP_HEADER_LOCATION]
	 */
	void redirect(const String& location) __deprecated
	{
		headers[HTTP_HEADER_LOCATION] = location;
	}

	/**
	 * @deprecated Use response.code = HTTP_STATUS_FORBIDDEN instead
	 */
	void forbidden() __deprecated
	{
		code = HTTP_STATUS_FORBIDDEN;
	}

	/**
	 * @deprecated Use response.code = HTTP_STATUS_NOT_FOUND instead
	 */
	void notFound() __deprecated
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

	/**
	 * @brief Send file by name
	 * @param fileName
	 * @param allowGzipFileCheck If true, check file extension to see if content commpressed
	 * @retval bool
	 */
	bool sendFile(String fileName, bool allowGzipFileCheck = true);

	/**
	 * @brief Parse and send template file
	 * @param newTemplateInstance
	 * @retval bool
	 */
	bool sendTemplate(TemplateStream* newTemplateInstance);

	/**
	 * @brief Build and send JSON string
	 *
	 * @deprecated use response.sendDataStream(stream, MIME_JSON) instead
	 */
	bool sendJsonObject(JsonObjectStream* newJsonStreamInstance) __deprecated;

	/** @brief Send data from the given stream object
	 *  @param newDataStream
	 *  @param type
	 *  @retval false on error
	 */
	bool sendDataStream(IDataSourceStream* newDataStream, enum MimeType type)
	{
		return sendDataStream(newDataStream, ContentType::toString(type));
	}

	/** @brief Send data from the given stream object
	 *  @param newDataStream
	 *  @param reqContentType
	 *  @retval on error returns false and stream will have been destroyed so any external
	 *  references to it must be invalidated.
	 *  @note all data is submitted via stream so called by internal routines
	 */
	bool sendDataStream(IDataSourceStream* newDataStream, const String& reqContentType = nullptr);

	/**
	 * @brief Get response body as a string
	 * @retval String
	 * @note Use with caution if response is large
	 */
	String getBody();

	/**
	 * @brief reset response so it can be re-used
	 */
	void reset();

	/**
	 * @brief Called by connection to specify where incoming response data is written
	 * @param buffer
	 */
	void setBuffer(ReadWriteStream* buffer);

	/**
	 * @brief release allocated stream memory
	 */
	void freeStreams();

private:
	void setStream(IDataSourceStream* stream);

public:
	unsigned code = HTTP_STATUS_OK; ///< The HTTP status response code
	HttpHeaders headers;
	ReadWriteStream* buffer = nullptr;   ///< Internal stream for storing strings and receiving responses
	IDataSourceStream* stream = nullptr; ///< The body stream
};

#endif /* _SMING_CORE_HTTP_RESPONSE_H_ */
