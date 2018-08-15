/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpServerConnection
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

/*
 *
 * 13/8/2018 (mikee47)
 *
 * 	Parser callback code revised (using macros) with handlers defined in non-static methods.
 * 	Simplifies code.
 *
 * 	Class constructor passed reference to ResourceTree and BodyParser to ensure they
 * 	are set. Accessor methods removed.
 *
 * 	getStatus method removed, use httpGetStatusText() in HttpCommon.
 */

#ifndef _SMING_CORE_HTTPSERVERCONNECTION_H_
#define _SMING_CORE_HTTPSERVERCONNECTION_H_

#include "../TcpClient.h"
#include "Delegate.h"

#include "HttpResource.h"
#include "HttpRequest.h"
#include "HttpBodyParser.h"

#ifndef HTTP_SERVER_EXPOSE_NAME
#define HTTP_SERVER_EXPOSE_NAME 1
#endif

#ifndef HTTP_SERVER_EXPOSE_DATE
#define HTTP_SERVER_EXPOSE_DATE 0
#endif

class HttpServerConnection;

typedef Delegate<void(HttpServerConnection& connection)> HttpServerConnectionDelegate;

class HttpServerConnection : public TcpClient {
public:
	HttpServerConnection(tcp_pcb* clientTcp, ResourceTree& resourceTree, BodyParsers& bodyParsers) :
		TcpClient(clientTcp, nullptr, nullptr),
		_resourceTree(resourceTree),
		_bodyParsers(bodyParsers)
	{
		// create parser ...
		http_parser_init(&_parser, HTTP_REQUEST);
		_parser.data = this;
	}

	virtual ~HttpServerConnection()
	{
		if (_resource)
			_resource->shutdown(*this);
	}

	void send();

	using TcpClient::send;

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

protected:
	virtual err_t onReceive(pbuf* buf);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void sendError(const String& message = nullptr, enum http_status code = HTTP_STATUS_BAD_REQUEST);
	virtual void onError(err_t err);

private:
	HTTP_PARSER_METHOD_0(HttpServerConnection, on_message_begin)
	HTTP_PARSER_METHOD_2(HttpServerConnection, on_url)
	HTTP_PARSER_METHOD_N(HttpServerConnection, on_status)
	HTTP_PARSER_METHOD_2(HttpServerConnection, on_header_field)
	HTTP_PARSER_METHOD_2(HttpServerConnection, on_header_value)
	HTTP_PARSER_METHOD_0(HttpServerConnection, on_headers_complete)
	HTTP_PARSER_METHOD_2(HttpServerConnection, on_body)
	HTTP_PARSER_METHOD_0(HttpServerConnection, on_message_complete)
	HTTP_PARSER_METHOD_N(HttpServerConnection, on_chunk_header)
	HTTP_PARSER_METHOD_N(HttpServerConnection, on_chunk_complete)

	HttpHeaders& prepareHeaders();

public:
	// << use to pass user data between requests
	void* userData = nullptr;

private:
	HttpConnectionState _state = eHCS_Ready;

	http_parser _parser;

	ResourceTree& _resourceTree; ///< Constructor gets reference to resource tree
	HttpResource* _resource;	 ///< Resource identified for current request

	HttpRequest _request;   ///< The current request
	HttpResponse _response; ///< The current response

	// Incoming request header processing
	bool _lastWasValue = true;
	String _lastData;
	HttpHeaderFieldName _currentField = hhfn_UNKNOWN;

	BodyParsers& _bodyParsers;			///< Constructor gets reference to body parsers to use
	HttpBodyParserDelegate _bodyParser; ///< Parser to use for current request
};

#endif /* _SMING_CORE_HTTPSERVERCONNECTION_H_ */
