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

/*
 * 13/8/2018 (mikee47)
 *
 * 	friend classes removed; instead they use public methods.
 * 	'stream' renamed to '_bodyStream' and retyped as IDataSourceStream as it's
 * 	read-only. For cases where an internal buffer is allocated, a separate
 * 	_buffer variable is used and referenced by _bodyStream. Allocation and
 * 	de-allocation of both are managed entirely within this class: no direct
 * 	access to either.
 *
 * 	sendFile() methods reworked to take advantage of new filesystem API.
 *
 * 	Methods added to perform standard maniuplation of response data and simplify
 * 	other code, also to eliminate the need to directly access member variables.
 */

#ifndef _SMING_CORE_HTTP_RESPONSE_H_
#define _SMING_CORE_HTTP_RESPONSE_H_

#include "HttpCommon.h"
#include "Data/Stream/TemplateStream.h"
#include "Data/HttpHeaders.h"
#include "FileSystem.h"


/*
 * Encapsulates response to a HTTP request, consisting of:
 *
 * 		result code
 * 		headers
 * 		body
 *
 * The body may be provided as a string or stream.
 */
class HttpResponse {
public:
	~HttpResponse()
	{
		reset();
	}

	void sendString(const String& text);

	void redirect(const String& location)
	{
		headers[hhfn_Location] = location;
	}

	/**
	 * @deprecated Use response.code = HTTP_STATUS_FORBIDDEN instead
	 */
	void forbidden()
	{
		code = HTTP_STATUS_FORBIDDEN;
	}

	/**
	 * @deprecated Use response.code = HTTP_STATUS_NOT_FOUND instead
	 */
	void notFound()
	{
		code = HTTP_STATUS_NOT_FOUND;
	}

	HttpResponse* setContentType(const String& type)
	{
		/*
		 * If content type is undefined, do not specify in header: client will have to auto-detect.
		 * The alternative is to specify application/octet-stream.
		 */
		if (type)
			headers[hhfn_ContentType] = type;
		else
			headers.remove(hhfn_ContentType);
		return this;
	}

	HttpResponse* setContentType(enum MimeType type)
	{
		setContentType(ContentType::toString(type));
		return this;
	}

	HttpResponse* setCookie(const String& name, const String& value)
	{
		headers[hhfn_SetCookie] = name + "=" + value;
		return this;
	}

	HttpResponse* setHeader(const String& name, const String& value)
	{
		headers[name] = value;
		return this;
	}

	HttpResponse* setCache(int maxAgeSeconds = 3600, bool isPublic = false);

	// Access-Control-Allow-Origin for AJAX from a different domain
	void setAllowCrossDomainOrigin(const String& controlAllowOrigin)
	{
		headers[hhfn_AccessControlAllowOrigin] = controlAllowOrigin;
	}

	// Send file by stat
	bool sendFile(const spiffs_stat& stat);

	// Send file by name
	bool sendFile(const String& fileName, bool allowGzipFileCheck = true);

	// @deprecated

	bool sendTemplate(TemplateStream* stream)
	{
		return sendDataStream(stream, ContentType::fromFileName(stream->name()));
	}

	// @end deprecated

	// Send Datastream, can be called with Classes derived from
	bool sendDataStream(IDataSourceStream* stream, enum MimeType type)
	{
		return sendDataStream(stream, ContentType::toString(type));
	}

	/** @brief Send data from the given stream object
	 *  @param stream
	 *  @param contentType
	 *  @retval on error returns false and stream will have been destroyed so any external
	 *  references to it must be invalidated.
	 *  @note all data is submitted via stream so called by internal routines
	 */
	bool sendDataStream(IDataSourceStream* stream, const String& contentType = nullptr);

	String getBody();

	void reset();

	/*
	 * Called by connection to store incoming response data.
	 * Return true on success, false on error.
	 */
	bool bodyReceived(const char* at, size_t length);

	/*
	 * Called by connection to specify where incoming response data is written.
	 * If this is null or not called then a circular buffer is used. See bodyReceived().
	 */
	void setResponseStream(ReadWriteStream* stream)
	{
		freeStreams();
		_buffer = stream;
		// So response body can be read out again
		_bodyStream = _buffer;
	}

	/** @brief Get the HTTP status line including line ending
	 *  @retval String
	 *  @note This is the first line in the response headers
	 */
	String getStatusLine();

	HttpHeaders& prepareHeaders();

	IDataSourceStream* getBodyStream();

	bool hasBody()
	{
		return _bodyStream != nullptr;
	}

	void freeStreams();

private:
	void setBodyStream(IDataSourceStream* stream);

public:
	unsigned code = 0;	///< The HTTP status response code
	HttpHeaders headers;

private:
	/*
	 * For sending strings, etc. the body stream refers to this object,
	 * and for receiving incoming responses.
	 */
	ReadWriteStream* _buffer = nullptr;
	IDataSourceStream* _bodyStream = nullptr;	///< The response body stream
};

#endif /* _SMING_CORE_HTTP_RESPONSE_H_ */
