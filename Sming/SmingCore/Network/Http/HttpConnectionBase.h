/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpConnectionBase.h
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#ifndef _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_BASE_H_
#define _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_BASE_H_

#include "../TcpClient.h"
#include "WString.h"
#include "HttpCommon.h"
#include "HttpResponse.h"
#include "HttpRequest.h"

/** @defgroup   HTTP base connection
 *  @brief      Provides http base used for client and server connections
 *  @ingroup    http
 *  @{
 */

/** @brief Re-assembles headers from fragments via onHeaderField / onHeaderValue callbacks */
class HttpHeaderBuilder
{
public:
	int onHeaderField(const char* at, size_t length)
	{
		if(lastWasValue) {
			// we are starting to process new header - setLength keeps allocated memory
			lastData.setLength(0);
			lastWasValue = false;
		}
		lastData.concat(at, length);

		return 0;
	}

	int onHeaderValue(HttpHeaders& headers, const char* at, size_t length)
	{
		if(!lastWasValue) {
			currentField = lastData;
			headers[currentField] = nullptr;
			lastWasValue = true;
		}
		headers[currentField].concat(at, length);
		return 0;
	}

	void reset()
	{
		lastWasValue = true;
		lastData = nullptr;
		currentField = nullptr;
	}

private:
	bool lastWasValue = true; ///< Indicates whether last callback was Field or Value
	String lastData;		  ///< Content of field or value, may be constructed over several callbacks
	String currentField;	  ///< Header field name
};

class HttpConnectionBase : public TcpClient
{
public:
	HttpConnectionBase(http_parser_type type, bool autoDestruct = false) : TcpClient(autoDestruct)
	{
		init(type);
	}

	HttpConnectionBase(tcp_pcb* connection, http_parser_type type) : TcpClient(connection, nullptr, nullptr)
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

protected:
	/** @brief Called after all headers have been received and processed */
	void resetHeaders();

	/** @brief Initializes the http parser for a specific type of HTTP message
	 * 	@param http_parser_type
	 */
	virtual void init(http_parser_type type);

	// HTTP parser methods
	/** @brief Called when a new incoming data is beginning to come
	 * 	@paran http_parser* parser
	 * 	@return 0 on success, non-0 on error
	 */
	virtual int onMessageBegin(http_parser* parser) = 0;

	/** @brief Called when the URL path is known
	 * 	@param String path
	 * 	@return 0 on success, non-0 on error
	 */
	virtual int onPath(const URL& uri)
	{
		return 0;
	}

	/** @brief Called when all headers are received
	 * 	@param HttpHeaders headers - the processed headers
	 * 	@return 0 on success, non-0 on error
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
	virtual bool onProtocolUpgrade(http_parser* parser)
	{
		return true;
	}

	virtual void onHttpError(http_errno error);

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
};

/** @} */
#endif /* _SMING_CORE_NETWORK_HTTP_HTTP_CONNECTION_BASE_H_ */
