/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpServer.h"

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpServerConnection.h"
#include "TcpClient.h"
#include "../Wiring/WString.h"

HttpServer::HttpServer()
{
	defaultHandler = NULL;

	// Default processing headers
	// Add more in you app!
	enableHeaderProcessing("Cookie");
	enableHeaderProcessing("Host");
	enableHeaderProcessing("Content-Type");
	enableHeaderProcessing("Content-Length");
}

HttpServer::~HttpServer()
{
}

TcpConnection* HttpServer::createClient(tcp_pcb *clientTcp)
{
	TcpConnection* con = new HttpServerConnection(this, clientTcp);
	con->setTimeOut(20);
	return con;
}

void HttpServer::enableHeaderProcessing(String headerName)
{
	processingHeaders.add(headerName);
}

void HttpServer::addPath(String path, HttpPathCallback callback)
{
	if (!path.startsWith("/"))
		path = "/" + path;
	debugf("'%s' registered", path.c_str());
	paths[path] = callback;
}

void HttpServer::setDefaultHandler(HttpPathCallback callback)
{
	defaultHandler = callback;
}

bool HttpServer::process(HttpServerConnection &connection, HttpRequest &request, HttpResponse &response)
{
	String path = request.getPath();
	if (path.length() > 1 && path.endsWith("/"))
		path = path.substring(0, path.length() - 1);

	if (paths.contains(path))
	{
		paths[path](request, response);
		return true;
	}

	if (defaultHandler != NULL)
	{
		debugf("Default server handler for: '%s'", path.c_str());
		defaultHandler(request, response);
		return true;
	}

	debugf("ERROR at server 404: '%s' not found", path.c_str());
	return false;
}

bool HttpServer::isHeaderProcessingEnabled(String name)
{
	for (int i = 0; i < processingHeaders.count(); i++)
		if (processingHeaders[i].equals(name))
			return true;

	return false;
}
