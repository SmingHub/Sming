/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServer.cpp
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpServer.h"
#include "TcpClient.h"
#include "WString.h"

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
		path.remove(path.length() - 1);
	}
	debug_i("'%s' registered", path.c_str());

	resourceTree[path] = new HttpCompatResource(callback);
}

void HttpServer::addPath(const String& path, const HttpResourceDelegate& onRequestComplete)
{
	HttpResource* resource = new HttpResource;
	resource->onRequestComplete = onRequestComplete;
	resourceTree[path] = resource;
}
