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

void HttpServer::configure(const HttpServerSettings& settings)
{
	this->settings = settings;
	if(settings.minHeapSize > -1) {
		minHeapSize = settings.minHeapSize;
	}
	maxConnections = settings.maxActiveConnections;

	if(settings.useDefaultBodyParsers) {
		setBodyParser(MIME_FORM_URL_ENCODED, formUrlParser);
	}

	setKeepAlive(settings.keepAliveSeconds);
}

TcpConnection* HttpServer::createClient(tcp_pcb* clientTcp)
{
	HttpServerConnection* con = new HttpServerConnection(clientTcp);
	con->setResourceTree(&paths);
	con->setBodyParsers(&bodyParsers);
	con->setCloseOnContentError(settings.closeOnContentError);

	return con;
}
