/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServer
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpServer.h"

#include "TcpClient.h"
#include "WString.h"

void HttpServer::configure(const HttpServerSettings& settings)
{
	if (settings.minHeapSize >= 0)
		_minHeapSize = settings.minHeapSize;

	if (settings.useDefaultBodyParsers)
		setBodyParser(ContentType::toString(MIME_FORM_URL_ENCODED), formUrlParser);

	setTimeOut(settings.keepAliveSeconds);
#ifdef ENABLE_SSL
	_sslSessionCacheSize = settings.sslSessionCacheSize;
#endif
}

void HttpServer::setBodyParser(const String& contentType, HttpBodyParserDelegate parser)
{
	_bodyParsers[contentType] = parser;
}

TcpConnection* HttpServer::createClient(tcp_pcb* clientTcp)
{
	return new HttpServerConnection(clientTcp, _resourceTree, _bodyParsers);
}

void HttpServer::addPath(String path, const HttpPathDelegate& callback)
{
	if (path.length() > 1 && path.endsWith("/"))
		path.remove(path.length() - 1);
	debug_i("'%s' registered", path.c_str());

	HttpCompatResource* resource = new HttpCompatResource(callback);
	if (resource)
		_resourceTree[path] = resource;
}

void HttpServer::setDefaultHandler(const HttpPathDelegate& callback)
{
	addPath("*", callback);
}

void HttpServer::addPath(const String& path, const HttpResourceDelegate& onRequestComplete)
{
	HttpResource* resource = new HttpResource;
	if (resource) {
		resource->onRequestComplete = onRequestComplete;
		_resourceTree[path] = resource;
	}
}

void HttpServer::addPath(const String& path, HttpResource* resource)
{
	_resourceTree[path] = resource;
}

void HttpServer::setDefaultResource(HttpResource* resource)
{
	addPath("*", resource);
}
