/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpResponse.h"

#include "HttpServerConnection.h"
#include "../DataSourceStream.h"

HttpResponse::HttpResponse()
{
	status = HttpStatusCode::OK;
	stream = NULL;
	headerSent = false;
	bodySent = false;
}

HttpResponse::~HttpResponse()
{
	if (stream != NULL)
		delete stream;
	stream = NULL;
}

void HttpResponse::switchingProtocols()
{
	status = HttpStatusCode::SwitchingProtocols;
}
void HttpResponse::badRequest()
{
	status = HttpStatusCode::BadRequest;
}
void HttpResponse::notFound()
{
	status = HttpStatusCode::NotFound;
}
void HttpResponse::forbidden()
{
	status = HttpStatusCode::Forbidden;
}
void HttpResponse::authorizationRequired()
{
	status = HttpStatusCode::Unauthorized;
}
void HttpResponse::redirect(String location /* = "" */)
{
	status = HttpStatusCode::Found;
	setHeader("Location", location);
}

String HttpResponse::getStatusName()
{
	return status;
}

int HttpResponse::getStatusCode()
{
	int p = status.indexOf(' ');
	if (status.length() == 0 || p == -1) return 0;

	return status.substring(0, p).toInt();
}

bool HttpResponse::hasBody()
{
	return stream != NULL || bodySent;
}

///

void HttpResponse::setContentType(const String type)
{
	setHeader("Content-Type", type);
}

void HttpResponse::setCookie(const String name, const String value)
{
	setHeader("Set-Cookie", name + "=" + value);
}

void HttpResponse::setCache(int maxAgeSeconds, bool isPublic /* = false */)
{
	String chache = String(isPublic ? "public" : "private") +", max-age=" + String(maxAgeSeconds) + ", must-revalidate";
	setHeader("Cache-Control", chache);
}

void HttpResponse::setAllowCrossDomainOrigin(String controlAllowOrigin)
{
	setHeader("Access-Control-Allow-Origin", controlAllowOrigin);
}

void HttpResponse::setHeader(const String name, const String value)
{
	responseHeaders[name] = value;
}

bool HttpResponse::hasHeader(const String name)
{
	return responseHeaders.contains(name);
}

///

void HttpResponse::sendString(const char* string)
{
	if (stream != NULL && stream->getStreamType() != eSST_Memory)
	{
		SYSTEM_ERROR("Stream already created");
		delete stream;
		stream = NULL;
	}

	if (stream == NULL)
		stream = new MemoryDataStream();

	MemoryDataStream *writable = (MemoryDataStream*)stream;
	writable->write((const uint8_t*)string, strlen(string));
}

void HttpResponse::sendString(String string)
{
	sendString(string.c_str());
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
		setHeader("Content-Encoding", "gzip");
	}
	else if (fileExist(fileName))
	{
		debugf("found %s", fileName.c_str());
		stream = new FileStream(fileName);
	}
	else
	{
		notFound();
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
		notFound();
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
	if (!hasHeader("Content-Type"))
		setContentType(ContentType::JSON);
}
///

void HttpResponse::sendHeader(HttpServerConnection &connection)
{
	if (headerSent) return;

	String top = "HTTP/1.1 " + status + "\r\n";
	connection.writeString(top.c_str(), TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
	for (int i = 0; i < responseHeaders.count(); i++)
	{
		String write = responseHeaders.keyAt(i) + ": " + responseHeaders.valueAt(i) + "\r\n";
		connection.writeString(write.c_str(), TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
	}
	connection.writeString("\r\n");
	headerSent = true;
}

bool HttpResponse::sendBody(HttpServerConnection &connection)
{
	if (!headerSent) sendHeader(connection);
	if (stream == NULL) return true;

	connection.write(stream);

	if (stream->isFinished())
	{
		connection.flush();
		bodySent = true;
		debugf("Stream completed");
		delete stream; // Free memory now!
		stream = NULL;
		return true;
	}
	else
		return false;
}
