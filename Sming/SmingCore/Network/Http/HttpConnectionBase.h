/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpConnectionBase
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTPCONNECTIONBASE_H_
#define _SMING_CORE_NETWORK_HTTP_HTTPCONNECTIONBASE_H_

#include "../TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Delegate.h"
#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"

class HttpConnectionBase : public TcpClient
{
public:
	HttpConnectionBase(http_parser_type type, bool autoDestruct = false);
	HttpConnectionBase(tcp_pcb* connection, http_parser_type type);
	virtual void reset();
	virtual void cleanup();
	virtual void setDefaultParser();

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

protected:
	void resetHeaders();

	virtual void init(http_parser_type type);

	// HTTP parser methods
	/**
	 * Called when a new incoming data is beginning to come
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	virtual int onMessageBegin(http_parser* parser) = 0;

	/**
	 * Called when the URL path is known
	 * @param String path
	 * @return 0 on success, non-0 on error
	 */
	virtual int onPath(const URL& uri)
	{
		return 0;
	}

	/**
	 * Called when all headers are received
	 * @param HttpHeaders headers - the processed headers
	 * @return 0 on success, non-0 on error
	 */
	virtual int onHeadersComplete(const HttpHeaders& headers) = 0;

#ifndef COMPACT_MODE
	virtual int onStatus(http_parser* parser)
	{
		return 0;
	}

	virtual int onChunkHeader(http_parser* parser)
	{
		return 0;
	}

	virtual int onChunkComplete(http_parser* parser)
	{
		return 0;
	}

#endif /* COMPACT MODE */

	/**
	 * Called when a piece of body data is received
	 * @param const char* at -  the data
	 * @paran size_t length
	 * @return 0 on success, non-0 on error
	 */
	virtual int onBody(const char* at, size_t length) = 0;

	/**
	 * Called when the incoming data is complete
	 * @paran http_parser* parser
	 * @return 0 on success, non-0 on error
	 */
	virtual int onMessageComplete(http_parser* parser) = 0;

	/**
	 * Called when the HTTP protocol should be upgraded
	 * @param http_parser* parser
	 * @return bool true on success
	 */
	virtual bool onProtocolUpgrade(http_parser* parser)
	{
		return true;
	}

	// TCP methods
	virtual bool onTcpReceive(TcpClient& client, char* data, int size);
	virtual void onError(err_t err);

private:
	static int staticOnMessageBegin(http_parser* parser);
	static int staticOnPath(http_parser* parser, const char* at, size_t length);
#ifndef COMPACT_MODE
	static int staticOnStatus(http_parser* parser, const char* at, size_t length);
#endif
	static int staticOnHeadersComplete(http_parser* parser);
	static int staticOnHeaderField(http_parser* parser, const char* at, size_t length);
	static int staticOnHeaderValue(http_parser* parser, const char* at, size_t length);
	static int staticOnBody(http_parser* parser, const char* at, size_t length);
#ifndef COMPACT_MODE
	static int staticOnChunkHeader(http_parser* parser);
	static int staticOnChunkComplete(http_parser* parser);
#endif
	static int staticOnMessageComplete(http_parser* parser);

protected:
	http_parser parser;
	static http_parser_settings parserSettings;
	static bool parserSettingsInitialized;
	bool lastWasValue = true;
	String lastData = "";
	String currentField = "";
	HttpHeaders incomingHeaders;
	HttpConnectionState state = eHCS_Ready;
};

#endif /* _SMING_CORE_NETWORK_HTTP_HTTPCONNECTIONBASE_H_ */
