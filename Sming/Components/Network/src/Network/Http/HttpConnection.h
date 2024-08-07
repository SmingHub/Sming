/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpConnection.h
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "../TcpClient.h"
#include "WString.h"
#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"
#include "HttpHeaderBuilder.h"

/** @ingroup	HTTP
 *  @brief      Provides http base used for client and server connections
 *  @{
 */

class HttpConnection : public TcpClient
{
public:
	HttpConnection(http_parser_type type, bool autoDestruct = false) : TcpClient(autoDestruct)
	{
		init(type);
	}

	HttpConnection(tcp_pcb* connection, http_parser_type type) : TcpClient(connection, nullptr, nullptr)
	{
		init(type);
	}

	virtual void reset()
	{
		resetHeaders();
	}

	virtual void cleanup()
	{
		reset();
	}

	virtual void setDefaultParser();

	using TcpConnection::getRemoteIp;
	using TcpConnection::getRemotePort;

	using TcpClient::send;

	/* Overridden by HttpClientConnection */
	virtual bool send(HttpRequest* request)
	{
		delete request;
		return false;
	}

	bool isActive();

	/**
	 * @brief Returns pointer to the current request
	 * @retval HttpRequest*
	 */
	virtual HttpRequest* getRequest() = 0;

	/**
	 * @brief Returns pointer to the current response
	 * @retval HttpResponse*
	 */
	HttpResponse* getResponse()
	{
		return &response;
	}

protected:
	/** @brief Called after all headers have been received and processed */
	void resetHeaders();

	/** @brief Initializes the http parser for a specific type of HTTP message
	 *  @param type
	 */
	virtual void init(http_parser_type type);

	// HTTP parser methods

	/** @brief Called when a new incoming data is beginning to come
	 * 	@param parser
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onMessageBegin(http_parser* parser) = 0;

	/** @brief Called when the URL path is known
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onPath(const Url&)
	{
		return 0;
	}

	/** @brief Called when all headers are received
	 * 	@param headers The processed headers
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onHeadersComplete(const HttpHeaders& headers) = 0;

#ifndef COMPACT_MODE
	virtual int onStatus(http_parser*)
	{
		return 0;
	}

	virtual int onChunkHeader(http_parser*)
	{
		return 0;
	}

	virtual int onChunkComplete(http_parser*)
	{
		return 0;
	}

#endif /* COMPACT MODE */

	/** @brief Called when a piece of body data is received
	 * 	@param at the data
	 * 	@param length
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onBody(const char* at, size_t length) = 0;

	/** @brief Called when the incoming data is complete
	 * 	@param parser
	 * 	@retval int 0 on success, non-0 on error
	 */
	virtual int onMessageComplete(http_parser* parser) = 0;

	/** @brief Called when the HTTP protocol should be upgraded
	 * 	@param parser
	 * 	@retval bool true on success
	 */
	virtual bool onProtocolUpgrade([[maybe_unused]] http_parser* parser)
	{
		return true;
	}

	/**
	 * @brief Called when there was an error.
	 * @param error - the error code
	 * @retval bool - false if the problem is not recoverable, true otherwise
	 */
	virtual bool onHttpError(HttpError error);

	// TCP methods
	virtual bool onTcpReceive(TcpClient& client, char* data, int size);

	void onError(err_t err) override;

private:
	// http_parser callback functions
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
	static const http_parser_settings parserSettings; ///< Callback table for parser
	HttpHeaderBuilder header;						  ///< Header construction
	HttpHeaders incomingHeaders;					  ///< Full set of incoming headers
	HttpConnectionState state = eHCS_Ready;

	HttpResponse response;
};

/** @} */
