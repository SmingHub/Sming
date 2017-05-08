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

#ifndef _SMING_CORE_HTTPSERVER_H_
#define _SMING_CORE_HTTPSERVER_H_

#include "TcpServer.h"
#include "../Wiring/WString.h"
#include "../Wiring/WHashMap.h"
#include "../Delegate.h"
#include "Http/HttpResponse.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResource.h"
#include "Http/HttpServerConnection.h"
#include "Http/HttpBodyParser.h"

typedef struct {
	int maxActiveConnections = 10; // << the maximum number of concurrent requests..
	int keepAliveSeconds = 5; // << the default seconds to keep the connection alive before closing it
	int minHeapSize = -1; // << defines the min heap size that is required to accept connection.
					      //  -1 - means use server default
	bool useDefaultBodyParsers = 1; // << if the default body parsers,  as form-url-encoded, should be used
#ifdef ENABLE_SSL
	int sslSessionCacheSize = 10; // << number of SSL session ids to cache. Setting this to 0 will disable SSL session resumption.
#endif
} HttpServerSettings;

class HttpServer: public TcpServer
{
	friend class HttpServerConnection;

public:
	HttpServer();
	HttpServer(HttpServerSettings settings);
	virtual ~HttpServer();

	/*
	 * @brief Allows changing the server configuration
	 */
	void configure(HttpServerSettings settings);

	void setBodyParser(const String& contentType, HttpBodyParserDelegate parser);

	/**
	 * @param String path URL path.
	 * @note Path should start with slash. Trailing slashes will be removed.
	 * @param HttpPathDelegate callback - the callback that will handle this path
	 */
	void addPath(String path, const HttpPathDelegate& callback);
	void addPath(const String& path, const HttpResourceDelegate& onRequestComplete);
	void addPath(const String& path, HttpResource* resource);

	void setDefaultHandler(const HttpPathDelegate& callback);
	void setDefaultResource(HttpResource* resource);


protected:
	virtual TcpConnection* createClient(tcp_pcb *clientTcp);
	virtual void onConnectionClose(TcpClient& connection, bool success);

protected:
#ifdef ENABLE_SSL
	int minHeapSize = 16384;
#endif

private:
	HttpServerSettings settings;
	ResourceTree resourceTree;
	BodyParsers bodyParsers;
};

#endif /* _SMING_CORE_HTTPSERVER_H_ */
