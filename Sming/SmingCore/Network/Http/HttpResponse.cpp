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

HttpResponse::~HttpResponse()
{
	if(stream != NULL) {
		delete stream;
		stream = NULL;
	}
}

HttpResponse* HttpResponse::setContentType(const String type)
{
	return setHeader("Content-Type", type);
}

HttpResponse* HttpResponse::setContentType(enum MimeType type)
{
	return setContentType(ContentType::toString(type));
}

HttpResponse* HttpResponse::setCookie(const String name, const String value)
{
	return setHeader("Set-Cookie", name + "=" + value);
}

HttpResponse* HttpResponse::setCache(int maxAgeSeconds, bool isPublic /* = false */)
{
	String chache = String(isPublic ? "public" : "private") +", max-age=" + String(maxAgeSeconds) + ", must-revalidate";
	return setHeader("Cache-Control", chache);
}

HttpResponse* HttpResponse::setAllowCrossDomainOrigin(String controlAllowOrigin)
{
	return setHeader("Access-Control-Allow-Origin", controlAllowOrigin);
}

HttpResponse* HttpResponse::setHeader(const String name, const String value)
{
	headers[name] = value;
	return this;
}

bool HttpResponse::sendString(const String& text)
{
	MemoryDataStream* memStream = new MemoryDataStream();
	if (memStream->write((const uint8_t*)text.c_str(), text.length()) != text.length()) {
		return false;
	}

	if (stream != NULL)
	{
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = NULL;
	}

	stream = memStream;

	return true;
}

bool HttpResponse::hasHeader(const String name)
{
	return headers.contains(name);
}

void HttpResponse::redirect(const String& location) {
	headers["Location"] = location;
}

bool HttpResponse::sendFile(String fileName, bool allowGzipFileCheck /* = true*/)
{
	if (stream != NULL)
	{
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = NULL;
	}

	String compressed = fileName + ".gz";
	if (allowGzipFileCheck && fileExist(compressed))
	{
		debugf("found %s", compressed.c_str());
		stream = new FileStream(compressed);
		headers["Content-Encoding"] = "gzip";
	}
	else if (fileExist(fileName))
	{
		debugf("found %s", fileName.c_str());
		stream = new FileStream(fileName);
	}
	else
	{
		code = HTTP_STATUS_NOT_FOUND;
		return false;
	}

	if (!hasHeader("Content-Type"))
	{
		const char *mime = ContentType::fromFullFileName(fileName);
		if (mime != NULL)
			setContentType(mime);
	}

	return true;
}


bool HttpResponse::sendTemplate(TemplateFileStream* newTemplateInstance)
{
	if (stream != NULL)
	{
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = NULL;
	}

	stream = newTemplateInstance;
	if (!newTemplateInstance->fileExist())
	{
		code = HTTP_STATUS_NOT_FOUND;
		delete stream;
		stream = NULL;
		return false;
	}

	if (!hasHeader("Content-Type"))
	{
		const char *mime = ContentType::fromFullFileName(newTemplateInstance->fileName());
		if (mime != NULL)
			setContentType(mime);
	}
	return true;
}

bool HttpResponse::sendJsonObject(JsonObjectStream* newJsonStreamInstance)
{
	if (stream != NULL)
	{
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = NULL;
	}

	stream = newJsonStreamInstance;
	if (!hasHeader("Content-Type")) {
		setContentType(MIME_JSON);
	}

	return true;
}

bool HttpResponse::sendDataStream( IDataSourceStream * newDataStream , String reqContentType /* = "" */)
{
    if (stream != NULL)
    {
        SYSTEM_ERROR("Stream already created");
        delete stream;
        stream = NULL;
    }
    if (reqContentType != "")
    {
        setContentType(reqContentType);
    }
    stream = newDataStream;

    return true;
}

