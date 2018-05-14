/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpConnection
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_CONNECTION_H_
#define _SMING_CORE_HTTP_CONNECTION_H_

#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "../TcpClient.h"
#include "Data/Stream/DataSourceStream.h"
#include "../../Data/Stream/MultipartStream.h"
#include "../../Services/DateTime/DateTime.h"

typedef SimpleConcurrentQueue<HttpRequest*, HTTP_REQUEST_POOL_SIZE> RequestQueue;

enum HttpClientMode
{
	eHCM_String = 0,
	eHCM_File, // << Deprecated! Use eHCM_Stream stream instead
	eHCM_Stream,
	eHCM_UserDefined // << Deprecated! If you supply onBody callback then the incoming body will be processed from the callback directly
};

class HttpConnection : protected TcpClient {
	friend class HttpClient;

public:
	HttpConnection(RequestQueue* queue);
	~HttpConnection();

	bool connect(const String& host, int port, bool useSsl = false, uint32_t sslOptions = 0);

	void send(HttpRequest* request);

	bool isActive();

	/**
	 * @brief Returns pointer to the current request
	 * @return HttpRequest*
	 */
	HttpRequest* getRequest();

	/**
	 * @brief Returns pointer to the current response
	 * @return HttpResponse*
	 */
	HttpResponse* getResponse();

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

	using TcpClient::close;

#ifdef ENABLE_SSL
	using TcpClient::getSsl;
#endif

	// Backported for compatibility reasons
	// @deprecated
	__forceinline int getResponseCode() { return code; }
	String getResponseHeader(String headerName, String defaultValue = "");
	HttpHeaders &getResponseHeaders();
	DateTime getLastModifiedDate(); // Last-Modified header
	DateTime getServerDate(); // Date header

	String getResponseString();
	// @enddeprecated

protected:
	void reset();

	virtual err_t onReceive(pbuf *buf);
	virtual err_t onProtocolUpgrade(http_parser* parser);
	virtual void onReadyToSendData(TcpConnectionEvent sourceEvent);
	virtual void onError(err_t err);

	void cleanup();

private:
	static int staticOnMessageBegin(http_parser* parser);
#ifndef COMPACT_MODE
	static int staticOnStatus(http_parser *parser, const char *at, size_t length);
#endif
	static int staticOnHeadersComplete(http_parser* parser);
	static int staticOnHeaderField(http_parser *parser, const char *at, size_t length);
	static int staticOnHeaderValue(http_parser *parser, const char *at, size_t length);
	static int staticOnBody(http_parser *parser, const char *at, size_t length);
#ifndef COMPACT_MODE
	static int staticOnChunkHeader(http_parser* parser);
	static int staticOnChunkComplete(http_parser* parser);
#endif
	static int staticOnMessageComplete(http_parser* parser);

	void sendRequestHeaders(HttpRequest* request);
	bool sendRequestBody(HttpRequest* request);

protected:
	HttpClientMode mode;
	String responseStringData;

	RequestQueue* waitingQueue;
	RequestQueue executionQueue;
	http_parser parser;
	static http_parser_settings parserSettings;
	static bool parserSettingsInitialized;
	HttpHeaders responseHeaders;

	int code = 0;
	bool lastWasValue = true;
	String lastData = "";
	String currentField  = "";
	HttpRequest* incomingRequest = NULL;
	HttpRequest* outgoingRequest = NULL;

private:
	HttpConnectionState state = eHCS_Ready;

private:
	HttpPartResult multipartProducer();
};

#endif /* _SMING_CORE_HTTP_CONNECTION_H_ */
