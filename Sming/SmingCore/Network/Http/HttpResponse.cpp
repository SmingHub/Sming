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

HttpResponse::~HttpResponse()
{
	delete stream;
	stream = nullptr;
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
	if(stream != nullptr && stream->getStreamType() != eSST_Memory) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	if(stream == nullptr) {
		stream = new MemoryDataStream();
	}

	MemoryDataStream* writable = static_cast<MemoryDataStream*>(stream);
	bool success = (writable->write((const uint8_t*)text.c_str(), text.length()) == text.length());

	return success;
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
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	String compressed = fileName + ".gz";
	if(allowGzipFileCheck && fileExist(compressed)) {
		debug_d("found %s", compressed.c_str());
		stream = new FileStream(compressed);
		headers[HTTP_HEADER_CONTENT_ENCODING] = _F("gzip");
	} else if(fileExist(fileName)) {
		debug_d("found %s", fileName.c_str());
		stream = new FileStream(fileName);
	} else {
		code = HTTP_STATUS_NOT_FOUND;
		return false;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String mime = ContentType::fromFullFileName(fileName);
		if(mime)
			setContentType(mime);
	}

	return true;
}

bool HttpResponse::sendTemplate(TemplateFileStream* newTemplateInstance)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	stream = newTemplateInstance;
	if(!newTemplateInstance->fileExist()) {
		code = HTTP_STATUS_NOT_FOUND;
		delete stream;
		stream = nullptr;
		return false;
	}

	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String mime = ContentType::fromFullFileName(newTemplateInstance->fileName());
		if(mime)
			setContentType(mime);
	}

	if(!headers.contains(HTTP_HEADER_TRANSFER_ENCODING) && stream->available() < 0) {
		headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	return true;
}

bool HttpResponse::sendJsonObject(JsonObjectStream* newJsonStreamInstance)
{
	if(stream != nullptr) {
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = nullptr;
	}

	stream = newJsonStreamInstance;
	if(!headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		setContentType(MIME_JSON);
	}

	return true;
}

bool HttpResponse::sendDataStream(ReadWriteStream* newDataStream, const String& reqContentType)
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

	return true;
}

String HttpResponse::getBody()
{
	if(stream == nullptr) {
		return "";
	}

	String ret;
	if(stream->available() != -1 && stream->getStreamType() == eSST_Memory) {
		MemoryDataStream* memory = (MemoryDataStream*)stream;
		char buf[1024];
		while(stream->available() > 0) {
			int available = memory->readMemoryBlock(buf, 1024);
			memory->seek(available);
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
	code = 0;
	headers.clear();
	delete stream;
	stream = nullptr;
}
