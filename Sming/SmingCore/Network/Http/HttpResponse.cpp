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
#include "../WebConstants.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/JsonObjectStream.h"
#include "Data/Stream/FileStream.h"
#include "Data/Stream/TemplateStream.h"

HttpResponse::~HttpResponse()
{
	freeStreams();
}

HttpResponse* HttpResponse::setContentType(const String& type)
{
	headers[HTTP_HEADER_CONTENT_TYPE] = type;
	return this;
}

HttpResponse* HttpResponse::setContentType(enum MimeType type)
{
	return setContentType(ContentType::toString(type));
}

HttpResponse* HttpResponse::setCookie(const String& name, const String& value)
{
	headers[HTTP_HEADER_SET_COOKIE] = name + '=' + value;
	return this;
}

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic /* = false */)
{
	String cache = isPublic ? F("public") : F("private");
	cache += F(", max-age=") + String(maxAgeSeconds) + F(", must-revalidate");
	headers[HTTP_HEADER_CACHE_CONTROL] = cache;
	return this;
}

HttpResponse* HttpResponse::setAllowCrossDomainOrigin(const String& controlAllowOrigin)
{
	headers[HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN] = controlAllowOrigin;
	return this;
}

HttpResponse* HttpResponse::setHeader(const String& name, const String& value)
{
	headers[name] = value;
	return this;
}

bool HttpResponse::sendString(const String& text)
{
	if(buffer == nullptr) {
		setBuffer(new MemoryDataStream());
		if(buffer == nullptr) {
			return false;
		}
	}

	return buffer->print(text) == text.length();
}

bool HttpResponse::hasHeader(const String& name)
{
	return headers.contains(name);
}

void HttpResponse::redirect(const String& location)
{
	headers[HTTP_HEADER_LOCATION] = location;
}

bool HttpResponse::sendFile(String fileName, bool allowGzipFileCheck /* = true*/)
{
	FileStream* fileStream;
	String compressed = fileName + ".gz";
	if(allowGzipFileCheck && fileExist(compressed)) {
		debug_d("found %s", compressed.c_str());
		fileStream = new FileStream(compressed);
		headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
	} else if(fileExist(fileName)) {
		debug_d("found %s", fileName.c_str());
		fileStream = new FileStream(fileName);
	} else {
		code = HTTP_STATUS_NOT_FOUND;
		return false;
	}

	String mime;
	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		mime = ContentType::fromFullFileName(fileName);
	}

	return sendDataStream(fileStream, mime);
}

bool HttpResponse::sendTemplate(TemplateStream* newTemplateInstance)
{
	if(!newTemplateInstance->isValid()) {
		code = HTTP_STATUS_NOT_FOUND;
		delete newTemplateInstance;
		return false;
	}

	String mime;
	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		mime = ContentType::fromFullFileName(newTemplateInstance->getName());
	}

	return sendDataStream(newTemplateInstance, mime);
}

void HttpResponse::setBuffer(ReadWriteStream* stream)
{
	freeStreams();
	buffer = stream;

	this->stream = buffer;
}

void HttpResponse::setBodyStream(IDataSourceStream* stream)
{
	if(this->stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		if(buffer == this->stream) {
			buffer = nullptr;
		}
		delete this->stream;
	}

	this->stream = stream;
}

void HttpResponse::freeStreams()
{
	if(buffer != nullptr) {
		if(buffer != stream) {
			debug_e("HttpResponse: buffer doesn't match stream");
			delete buffer;
		}
		buffer = nullptr;
	}

	delete stream;
	stream = nullptr;
}

bool HttpResponse::sendJsonObject(JsonObjectStream* newJsonStreamInstance)
{
	return sendDataStream(newJsonStreamInstance, MIME_JSON);
}

bool HttpResponse::sendDataStream(IDataSourceStream* newDataStream, const String& reqContentType)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}
	if(reqContentType) {
		setContentType(reqContentType);
	}
	stream = newDataStream;

	if(!headers.contains(HTTP_HEADER_TRANSFER_ENCODING) && stream->available() < 0) {
		headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	return true;
}

String HttpResponse::getBody()
{
	if(stream == nullptr) {
		return nullptr;
	}

	String ret;
	if(stream->available() > 0 && stream->getStreamType() == eSST_Memory) {
		char buf[1024];
		while(stream->available() > 0) {
			int available = stream->readMemoryBlock(buf, 1024);
			stream->seek(available);
			ret += String(buf, available);
			if(available < 1024) {
				break;
			}
		}
	}
	return ret;
}

void HttpResponse::reset()
{
	code = HTTP_STATUS_OK;
	headers.clear();
	freeStreams();
}
