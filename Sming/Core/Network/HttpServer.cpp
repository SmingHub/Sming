/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
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

#ifdef ENABLE_HTTP_SERVER_MULTIPART
#include <MultipartParser/MultipartParser.h>
#endif

void HttpServer::configure(const HttpServerSettings& settings)
{
	this->settings = settings;
	if(settings.minHeapSize > -1) {
		minHeapSize = settings.minHeapSize;
	}

	if(settings.useDefaultBodyParsers) {
		setBodyParser(ContentType::toString(MIME_FORM_URL_ENCODED), formUrlParser);
#ifdef ENABLE_HTTP_SERVER_MULTIPART
		setBodyParser(ContentType::toString(MIME_FORM_MULTIPART), formMultipartParser);
#endif
	}

	setKeepAlive(settings.keepAliveSeconds);
#ifdef ENABLE_SSL
	sslSessionCacheSize = settings.sslSessionCacheSize;
#endif
}

TcpConnection* HttpServer::createClient(tcp_pcb* clientTcp)
{
	HttpServerConnection* con = new HttpServerConnection(clientTcp);
	con->setResourceTree(&paths);
	con->setBodyParsers(&bodyParsers);

	return con;
}
