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

#include "HttpResponse.h"
#include "WebConstants.h"
#include "Data/CircularBuffer.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/ChunkedStream.h"
#include "Data/Stream/FileStream.h"


// When receiving response body using internal buffer this is the size we allocate
#define DEFAULT_RESPONSE_BUFFER_SIZE 1024

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic /* = false */)
{
	String cache = isPublic ? _F("public") : _F("private");
	cache += _F(", max-age=");
	cache += String(maxAgeSeconds);
	cache += _F(", must-revalidate");
	headers[hhfn_CacheControl] = cache;
	return this;
}

void HttpResponse::sendString(const String& text)
{
	if (_buffer) {
		_buffer->print(text);
	}
	else {
		_buffer = new MemoryDataStream();
		_buffer->print(text);
		if (!sendDataStream(_buffer))
			_buffer = nullptr;
	}
}

bool HttpResponse::sendFile(const spiffs_stat& stat)
{
	String name = (const char*)stat.name;
	if (name.endsWith(F(".gz"))) {
		headers[hhfn_ContentEncoding] = F("gzip");
		// Content type is determined by uncompressed file extension
		name.setLength(name.length() - 3);
	}

	return sendDataStream(new FileStream(stat), ContentType::fromFileName(name));
}

bool HttpResponse::sendFile(const String& fileName, bool allowGzipFileCheck /* = true*/)
{
	spiffs_stat stat;

	if (allowGzipFileCheck) {
		String compressed = fileName + _F(".gz");
		if (fileStats(compressed, &stat) >= 0) {
			debug_d("found %s", compressed.c_str());
			return sendFile(stat);
		}
	}

	if (fileStats(fileName, &stat) >= 0) {
		debug_d("found %s", fileName.c_str());
		return sendFile(stat);
	}

	code = HTTP_STATUS_NOT_FOUND;
	return false;
}

void HttpResponse::setBodyStream(IDataSourceStream* stream)
{
	if (_bodyStream) {
		SYSTEM_ERROR("Stream already created");
		if (_buffer == _bodyStream)
			_buffer = nullptr;
		delete _bodyStream;
	}

	_bodyStream = stream;
}

String HttpResponse::getStatusLine()
{
	return F("HTTP/1.1 ") + String(code) + " " + httpGetStatusText(code) + "\r\n";
}

HttpHeaders& HttpResponse::prepareHeaders()
{
	if (_bodyStream) {
		int length = _bodyStream->available();
		if (length >= 0)
			headers[hhfn_ContentLength] = String(length);
	}
	else if (!headers.contains(hhfn_ContentLength))
		headers[hhfn_ContentLength] = "0";

#ifndef DISABLE_HTTPSRV_ETAG
	if (_bodyStream && !headers.contains(hhfn_ETag)) {
		String tag = _bodyStream->id();
		if (tag)
			headers[hhfn_ETag] = String('"' + tag + '"');
	}
#endif

	return headers;
}

IDataSourceStream* HttpResponse::getBodyStream()
{
	auto s = _bodyStream;
	_bodyStream = nullptr;

	if (s && headers[hhfn_TransferEncoding] == F("chunked"))
		return new ChunkedStream(s);
	else
		return s;
}

void HttpResponse::freeStreams()
{
	if (_buffer) {
		/*
		 * If assigned, buffer must be referenced by _bodyStream otherwise we have a bug
		 * somewhere in this class.
		 */
		assert(_buffer == _bodyStream);
		_buffer = nullptr;
	}

	delete _bodyStream;
	_bodyStream = nullptr;
}

bool HttpResponse::sendDataStream(IDataSourceStream* stream, const String& reqContentType)
{
	if (!stream->isValid()) {
		debug_e("HttpResponse::sendDataStream() stream invalid, destroying");
		code = HTTP_STATUS_NOT_FOUND;
		delete stream;
		return false;
	}

	setBodyStream(stream);

	if (reqContentType)
		setContentType(reqContentType);

	//!! This messes up websocket response so clearly doesn't apply to all transfers...

	// If body size cannot be determined then we must use 'chunk' transfer encoding
	if (_bodyStream->available() < 0)
		headers[hhfn_TransferEncoding] = F("chunked");

	return true;
}

String HttpResponse::getBody()
{
	if (!_bodyStream || _bodyStream->getStreamType() != eSST_Memory)
		return nullptr;

	int len = _bodyStream->available();
	if (len <= 0)
		return nullptr;

	String s;
	if (s.setLength(len))
		_bodyStream->readMemoryBlock(s.begin(), len);
	return s;
}

void HttpResponse::reset()
{
	code = HTTP_STATUS_OK;
	headers.clear();
	freeStreams();
}

bool HttpResponse::bodyReceived(const char* at, size_t length)
{
	if (!_buffer) {
		_buffer = new CircularBuffer(DEFAULT_RESPONSE_BUFFER_SIZE);
		setBodyStream(_buffer);
	}

	unsigned res = _buffer->write((const uint8_t*)at, length);
	if (res != length) {
		// unable to write the requested bytes - stop here...
		freeStreams();
		return false;
	}

	return true;
}
