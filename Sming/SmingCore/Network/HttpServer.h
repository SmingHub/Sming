/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTPSERVER_H_
#define _SMING_CORE_HTTPSERVER_H_

#include "TcpServer.h"
#include "../../Wiring/WHashMap.h"
#include "../../Wiring/WVector.h"
#include "../Delegate.h"

class String;
class HttpServerConnection;
class HttpRequest;
class HttpResponse;

//typedef void (*HttpPathCallback)(HttpRequest &request, HttpResponse &response);
typedef Delegate<void(HttpRequest&, HttpResponse&)> HttpPathCallback;

class HttpServer: public TcpServer
{
public:
	HttpServer();
	virtual ~HttpServer();

	void enableHeaderProcessing(String headerName);
	bool isHeaderProcessingEnabled(String name);

	void addPath(String path, HttpPathCallback callback);
	void setDefaultHandler(HttpPathCallback callback);
	bool process(HttpServerConnection &connection, HttpRequest &request, HttpResponse &response);

protected:
	virtual TcpConnection* createClient(tcp_pcb *clientTcp);

private:
	HttpPathCallback defaultHandler;
	Vector<String> processingHeaders;
	HashMap<String, HttpPathCallback> paths;
};

#endif /* _SMING_CORE_HTTPSERVER_H_ */
