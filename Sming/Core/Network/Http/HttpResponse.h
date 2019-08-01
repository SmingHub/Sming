/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpResponse.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpCommon.h"
#include "Data/Stream/ReadWriteStream.h"
#include "HttpHeaders.h"
#include "FileSystem.h"

class HttpResponse
{
public:
	~HttpResponse()
	{
		freeStreams();
	}

	bool sendString(const String& text);

	/**
	 * @deprecated Use `headers.contains()` instead
	 */
	bool hasHeader(const String& name) SMING_DEPRECATED
	{
		return headers.contains(name);
	}

	/**
	 * @deprecated Use `headers[HTTP_HEADER_LOCATION]` instead
	 */
	void redirect(const String& location) SMING_DEPRECATED
	{
		headers[HTTP_HEADER_LOCATION] = location;
	}

	/**
	 * @deprecated Use `response.code = HTTP_STATUS_FORBIDDEN` instead
	 */
	void forbidden() SMING_DEPRECATED
	{
		code = HTTP_STATUS_FORBIDDEN;
	}

	/**
	 * @deprecated Use `response.code = HTTP_STATUS_NOT_FOUND` instead
	 */
	void notFound() SMING_DEPRECATED
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
	bool sendFile(const String& fileName, bool allowGzipFileCheck = true);

	/**
	 * @brief Parse and send template file
	 * @param newTemplateInstance
	 * @retval bool
	 * @deprecated Use `sendNamedStream()` instead
	 */
	bool sendTemplate(IDataSourceStream* newTemplateInstance) SMING_DEPRECATED
	{
		return sendNamedStream(newTemplateInstance);
	}

	/**
	 * @brief Parse and send stream, using the name to determine the content type
	 * @param newDataStream If not set already, the contentType will be obtained from the name of this stream
	 * @retval bool
	 */
	bool sendNamedStream(IDataSourceStream* newDataStream);

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
