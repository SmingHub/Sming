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
#include "../Wiring/WString.h"

void HttpServer::configure(const HttpServerSettings& settings)
{
	this->settings = settings;
	if(settings.minHeapSize > -1) {
		minHeapSize = settings.minHeapSize;
	}

	if(settings.useDefaultBodyParsers) {
		setBodyParser(ContentType::toString(MIME_FORM_URL_ENCODED), formUrlParser);
	}

	setKeepAlive(settings.keepAliveSeconds);
#ifdef ENABLE_SSL
	sslSessionCacheSize = settings.sslSessionCacheSize;
#endif
}

HttpServer::~HttpServer()
{
	for(unsigned i = 0; i < resourceTree.count(); i++) {
		delete resourceTree.valueAt(i);
	}
}

void HttpServer::setBodyParser(const String& contentType, HttpBodyParserDelegate parser)
{
	bodyParsers[contentType] = parser;
}

TcpConnection* HttpServer::createClient(tcp_pcb* clientTcp)
{
	HttpServerConnection* con = new HttpServerConnection(clientTcp);
	con->setResourceTree(&resourceTree);
	con->setBodyParsers(&bodyParsers);

	return con;
}

void HttpServer::addPath(String path, const HttpPathDelegate& callback)
{
	if(path.length() > 1 && path.endsWith("/")) {
		path = path.substring(0, path.length() - 1);
	}
	debug_i("'%s' registered", path.c_str());

	HttpCompatResource* resource = new HttpCompatResource(callback);
	resourceTree[path] = resource;
}

void HttpServer::setDefaultHandler(const HttpPathDelegate& callback)
{
	addPath("*", callback);
}

void HttpServer::addPath(const String& path, const HttpResourceDelegate& onRequestComplete)
{
	HttpResource* resource = new HttpResource;
	resource->onRequestComplete = onRequestComplete;
	resourceTree[path] = resource;
}

void HttpServer::addPath(const String& path, HttpResource* resource)
{
	resourceTree[path] = resource;
}

void HttpServer::setDefaultResource(HttpResource* resource)
{
	addPath("*", resource);
}
