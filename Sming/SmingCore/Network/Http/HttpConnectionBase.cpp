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

#include "HttpConnectionBase.h"

bool HttpConnectionBase::parserSettingsInitialized = false;
http_parser_settings HttpConnectionBase::parserSettings;

HttpConnectionBase::HttpConnectionBase(http_parser_type type, bool autoDestruct) : TcpClient(autoDestruct)
{
	init(type);
}

HttpConnectionBase::HttpConnectionBase(tcp_pcb* connection, http_parser_type type) : TcpClient(connection, 0, 0)
{
	init(type);
}

void HttpConnectionBase::init(http_parser_type type)
{
	http_parser_init(&parser, type);
	parser.data = (void*)this;

	if(!parserSettingsInitialized) {
		memset(&parserSettings, 0, sizeof(parserSettings));

		// Notification callbacks: on_message_begin, on_headers_complete, on_message_complete.
		parserSettings.on_message_begin = staticOnMessageBegin;
		parserSettings.on_headers_complete = staticOnHeadersComplete;
		parserSettings.on_message_complete = staticOnMessageComplete;

#ifndef COMPACT_MODE
		parserSettings.on_chunk_header = staticOnChunkHeader;
		parserSettings.on_chunk_complete = staticOnChunkComplete;
#endif
		parserSettings.on_url = staticOnPath;

// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
#ifndef COMPACT_MODE
		parserSettings.on_status = staticOnStatus;
#endif
		parserSettings.on_header_field = staticOnHeaderField;
		parserSettings.on_header_value = staticOnHeaderValue;
		parserSettings.on_body = staticOnBody;

		parserSettingsInitialized = true;
	}

	setDefaultParser();
}

void HttpConnectionBase::setDefaultParser()
{
	TcpClient::setReceiveDelegate(TcpClientDataDelegate(&HttpConnectionBase::onTcpReceive, this));
}

void HttpConnectionBase::resetHeaders()
{
	lastWasValue = true;
	lastData = "";
	currentField = "";
	incomingHeaders.clear();
}

void HttpConnectionBase::reset()
{
	resetHeaders();
}

void HttpConnectionBase::cleanup()
{
	reset();
}

int HttpConnectionBase::staticOnMessageBegin(http_parser* parser)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	connection->reset();
	return connection->onMessageBegin(parser);
}

int HttpConnectionBase::staticOnPath(http_parser* parser, const char* at, size_t length)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	return connection->onPath(URL(String(at, length)));
}

#ifndef COMPACT_MODE
int HttpConnectionBase::staticOnStatus(http_parser* parser, const char* at, size_t length)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	return connection->onStatus(parser);
}

int HttpConnectionBase::staticOnChunkHeader(http_parser* parser)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	return connection->onChunkHeader(parser);
}

int HttpConnectionBase::staticOnChunkComplete(http_parser* parser)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	return connection->onChunkComplete(parser);
}
#endif

int HttpConnectionBase::staticOnHeaderField(http_parser* parser, const char* at, size_t length)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(connection->lastWasValue) {
		// we are starting to process new header
		connection->lastData = "";
		connection->lastWasValue = false;
	}
	connection->lastData += String(at, length);

	return 0;
}

int HttpConnectionBase::staticOnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(!connection->lastWasValue) {
		connection->currentField = connection->lastData;
		connection->incomingHeaders[connection->currentField] = nullptr;
		connection->lastWasValue = true;
	}
	connection->incomingHeaders[connection->currentField] += String(at, length);

	return 0;
}

int HttpConnectionBase::staticOnHeadersComplete(http_parser* parser)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

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
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	return connection->onBody(at, length);
}

int HttpConnectionBase::staticOnMessageComplete(http_parser* parser)
{
	HttpConnectionBase* connection = static_cast<HttpConnectionBase*>(parser->data);
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

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
