/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpConnectionBase.cpp
 *
 * @author: 2018 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpConnectionBase.h"

/** @brief http_parser function table
 *  @note stored in flash memory; as it is word-aligned it can be accessed directly
 *  Notification callbacks: on_message_begin, on_headers_complete, on_message_complete
 *  Data callbacks: on_url, (common) on_header_field, on_header_value, on_body
 */
const http_parser_settings HttpConnectionBase::parserSettings PROGMEM = {
	.on_message_begin = staticOnMessageBegin,
	.on_url = staticOnPath,
#ifdef COMPACT_MODE
	.on_status = nullptr,
#else
	.on_status = staticOnStatus,
#endif
	.on_header_field = staticOnHeaderField,
	.on_header_value = staticOnHeaderValue,
	.on_headers_complete = staticOnHeadersComplete,
	.on_body = staticOnBody,
	.on_message_complete = staticOnMessageComplete,
#ifdef COMPACT_MODE
	.on_chunk_header = nullptr,
	.on_chunk_complete = nullptr,
#else
	.on_chunk_header = staticOnChunkHeader,
	.on_chunk_complete = staticOnChunkComplete
#endif
};

/** @brief Boilerplate code for http_parser callbacks
 *  @note Obtain connection object and check it
 */
#define GET_CONNECTION()                                                                                               \
	auto connection = static_cast<HttpConnectionBase*>(parser->data);                                                  \
	if(connection == nullptr) {                                                                                        \
		return -1;                                                                                                     \
	}

void HttpConnectionBase::init(http_parser_type type)
{
	http_parser_init(&parser, type);
	parser.data = this;
	setDefaultParser();
}

void HttpConnectionBase::setDefaultParser()
{
	TcpClient::setReceiveDelegate(TcpClientDataDelegate(&HttpConnectionBase::onTcpReceive, this));
}

void HttpConnectionBase::resetHeaders()
{
	header.reset();
	incomingHeaders.clear();
}

int HttpConnectionBase::staticOnMessageBegin(http_parser* parser)
{
	GET_CONNECTION()

	connection->reset();
	return connection->onMessageBegin(parser);
}

int HttpConnectionBase::staticOnPath(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->onPath(URL(String(at, length)));
}

#ifndef COMPACT_MODE
int HttpConnectionBase::staticOnStatus(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->onStatus(parser);
}

int HttpConnectionBase::staticOnChunkHeader(http_parser* parser)
{
	GET_CONNECTION()

	return connection->onChunkHeader(parser);
}

int HttpConnectionBase::staticOnChunkComplete(http_parser* parser)
{
	GET_CONNECTION()

	return connection->onChunkComplete(parser);
}
#endif

int HttpConnectionBase::staticOnHeaderField(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->header.onHeaderField(at, length);
}

int HttpConnectionBase::staticOnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->header.onHeaderValue(connection->incomingHeaders, at, length);
}

int HttpConnectionBase::staticOnHeadersComplete(http_parser* parser)
{
	GET_CONNECTION()

	debug_d("The headers are complete");

	/* Callbacks should return non-zero to indicate an error. The parser will
	 * then halt execution.
	 *
	 * The one exception is on_headers_complete. In a HTTP_RESPONSE parser
	 * returning '1' from on_headers_complete will tell the parser that it
	 * should not expect a body. This is used when receiving a response to a
	 * HEAD request which may contain 'Content-Length' or 'Transfer-Encoding:
	 * chunked' headers that indicate the presence of a body.
	 *
	 * Returning `2` from on_headers_complete will tell parser that it should not
	 * expect neither a body nor any further responses on this connection. This is
	 * useful for handling responses to a CONNECT request which may not contain
	 * `Upgrade` or `Connection: upgrade` headers.
	 */
	int error = connection->onHeadersComplete(connection->incomingHeaders);
	connection->resetHeaders();

	return error;
}

int HttpConnectionBase::staticOnBody(http_parser* parser, const char* at, size_t length)
{
	GET_CONNECTION()

	return connection->onBody(at, length);
}

int HttpConnectionBase::staticOnMessageComplete(http_parser* parser)
{
	GET_CONNECTION()

	int error = connection->onMessageComplete(parser);
	connection->reset();

	return error;
}

void HttpConnectionBase::onHttpError(http_errno error)
{
	debug_e("HTTP parser error: %s", httpGetErrorName(error).c_str());
}

bool HttpConnectionBase::onTcpReceive(TcpClient& client, char* data, int size)
{
	int parsedBytes = http_parser_execute(&parser, &parserSettings, data, size);
	if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
		// we ran into trouble - abort the connection
		onHttpError(HTTP_PARSER_ERRNO(&parser));
		return false;
	}

	if(parser.upgrade) {
		return onProtocolUpgrade(&parser);
	} else if(parsedBytes != size) {
		return false;
	}

	return true;
}

void HttpConnectionBase::onError(err_t err)
{
	cleanup();
	TcpClient::onError(err);
}
