/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServer.h
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/** @defgroup   httpserver HTTP server
 *  @brief      Provides powerful HTTP/S + Websocket server
 *  @ingroup    tcpserver
 *  @{
 */

#ifndef _SMING_CORE_NETWORK_HTTP_SERVER_H_
#define _SMING_CORE_NETWORK_HTTP_SERVER_H_

#include "TcpServer.h"
#include "WString.h"
#include "Delegate.h"
#include "Http/HttpResource.h"
#include "Http/HttpServerConnection.h"
#include "Http/HttpBodyParser.h"

typedef struct {
	int maxActiveConnections = 10; ///< maximum number of concurrent requests..
	int keepAliveSeconds = 0;	  ///< default seconds to keep the connection alive before closing it
	int minHeapSize = -1;		   ///< min heap size that is required to accept connection, -1 means use server default
	bool useDefaultBodyParsers = 1; ///< if the default body parsers,  as form-url-encoded, should be used
#ifdef ENABLE_SSL
	int sslSessionCacheSize =
		10; ///< number of SSL session ids to cache. Setting this to 0 will disable SSL session resumption.
#endif
} HttpServerSettings;

class HttpServer : public TcpServer
{
public:
	HttpServer()
	{
		settings.keepAliveSeconds = 2;
		configure(settings);
	}

	HttpServer(const HttpServerSettings& settings)
	{
		configure(settings);
	}

	/**
	 * @brief Allows changing the server configuration
	 */
	void configure(const HttpServerSettings& settings);

	/**
	 * @briefs Allows content-type specific parsing of the body based on content-type.
	 *
	 * @param const String& contentType. Can be full content-type like 'application/json', or 'application/*'  or '*'.
	 * 						If there is exact match for the content-type wildcard content-types will not be used.
	 * 						There can be only one catch-all '*' body parser and that will be the last registered
	 *
	 * @param  HttpBodyParserDelegate parser
	 */
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
	TcpConnection* createClient(tcp_pcb* clientTcp) override;

private:
	HttpServerSettings settings;
	ResourceTree resourceTree;
	BodyParsers bodyParsers;
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_SERVER_H_ */
