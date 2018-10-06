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

#include "HttpServerConnection.h"

#include "HttpServer.h"
#include "TcpServer.h"
#include "WebConstants.h"
#include "../../Data/Stream/ChunkedStream.h"

bool HttpServerConnection::parserSettingsInitialized = false;
http_parser_settings HttpServerConnection::parserSettings;

HttpServerConnection::HttpServerConnection(tcp_pcb* clientTcp) : TcpClient(clientTcp, 0, 0), state(eHCS_Ready)
{
	// create parser ...
	http_parser_init(&parser, HTTP_REQUEST);
	parser.data = (void*)this;

	if(!parserSettingsInitialized) {
		memset(&parserSettings, 0, sizeof(parserSettings));
		// Notification callbacks: on_message_begin, on_headers_complete, on_message_complete.
		parserSettings.on_message_begin = staticOnMessageBegin;
		parserSettings.on_headers_complete = staticOnHeadersComplete;
		parserSettings.on_message_complete = staticOnMessageComplete;

		// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
		parserSettings.on_url = staticOnPath;
		parserSettings.on_header_field = staticOnHeaderField;
		parserSettings.on_header_value = staticOnHeaderValue;
		parserSettings.on_body = staticOnBody;
		parserSettingsInitialized = true;
	}
}

HttpServerConnection::~HttpServerConnection()
{
	if(this->resource) {
		this->resource->shutdown(*this);
	}
}

void HttpServerConnection::setResourceTree(ResourceTree* resourceTree)
{
	this->resourceTree = resourceTree;
}

void HttpServerConnection::setBodyParsers(BodyParsers* bodyParsers)
{
	this->bodyParsers = bodyParsers;
}

int HttpServerConnection::staticOnMessageBegin(http_parser* parser)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	// Reset Response ...
	connection->response.code = 200;
	connection->response.headers.clear();
	if(connection->response.stream != nullptr) {
		delete connection->response.stream;
		connection->response.stream = nullptr;
	}

	connection->state = eHCS_Ready;

	// ... and Request
	// TODO:
	connection->request.setMethod((const HttpMethod)parser->method);

	// and temp data...
	connection->requestHeaders.clear();
	connection->bodyParser = 0;

	return 0;
}

int HttpServerConnection::staticOnPath(http_parser* parser, const char* at, size_t length)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	// TODO: find the most suitable path..

	String path = String(at, length);
	if(path.length() > 1 && path.endsWith("/")) {
		path = path.substring(0, path.length() - 1);
	}

	connection->request.setURL(path);

	if(connection->resourceTree == nullptr) {
		debug_e("ERROR: HttpServerConnection: The resource tree is not set!");

		return -1;
	}

	if(connection->resourceTree->contains(connection->request.uri.Path)) {
		connection->resource = (*connection->resourceTree)[connection->request.uri.Path];
	} else if(connection->resourceTree->contains("*")) {
		connection->resource = (*connection->resourceTree)["*"];
	}

	return 0;
}

int HttpServerConnection::staticOnMessageComplete(http_parser* parser)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	// we are finished with this request
	int hasError = 0;
	if(HTTP_PARSER_ERRNO(parser) != HPE_OK) {
		connection->sendError(httpGetErrorName(HTTP_PARSER_ERRNO(parser)));
		return 0;
	}

	if(connection->bodyParser) {
		connection->bodyParser(connection->request, nullptr, PARSE_DATAEND);
	}

	if(connection->resource != nullptr && connection->resource->onRequestComplete) {
		hasError = connection->resource->onRequestComplete(*connection, connection->request, connection->response);
	}

	connection->send();

	if(connection->request.responseStream != nullptr) {
		delete connection->request.responseStream;
		connection->request.responseStream = nullptr;
	}

	return hasError;
}

int HttpServerConnection::staticOnHeadersComplete(http_parser* parser)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
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
	int error = 0;
	connection->request.setHeaders(connection->requestHeaders);

	connection->lastWasValue = true;
	connection->lastData = "";
	connection->currentField = "";
	connection->requestHeaders.clear();

	if(connection->resource != nullptr && connection->resource->onHeadersComplete) {
		error = connection->resource->onHeadersComplete(*connection, connection->request, connection->response);
	}

	if(!error && connection->request.method == HTTP_HEAD) {
		error = 1;
	}

	if(connection->request.headers.contains(HTTP_HEADER_CONTENT_TYPE)) {
		String contentType = connection->request.headers[HTTP_HEADER_CONTENT_TYPE];
		int endPos = contentType.indexOf(';');
		if(endPos != -1) {
			contentType = contentType.substring(0, endPos);
		}

		String majorType = contentType.substring(0, contentType.indexOf('/'));
		majorType += "/*";

		// Content-Type for exact type: application/json
		// Wildcard type for application: application/*
		// Wildcard type for the rest*

		Vector<String> types;
		types.add(contentType);
		types.add(majorType);
		types.add("*");

		for(unsigned i = 0; i < types.count(); i++) {
			if(connection->bodyParsers->contains(types.at(i))) {
				connection->bodyParser = (*connection->bodyParsers)[types.at(i)];
				break;
			}
		}

		if(connection->bodyParser) {
			connection->bodyParser(connection->request, nullptr, -1);
		}
	}

	return error;
}

int HttpServerConnection::staticOnHeaderField(http_parser* parser, const char* at, size_t length)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
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

int HttpServerConnection::staticOnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(!connection->lastWasValue) {
		connection->currentField = connection->lastData;
		connection->requestHeaders[connection->currentField] = nullptr;
		connection->lastWasValue = true;
	}
	connection->requestHeaders[connection->currentField] += String(at, length);

	return 0;
}

int HttpServerConnection::staticOnBody(http_parser* parser, const char* at, size_t length)
{
	HttpServerConnection* connection = (HttpServerConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(connection->bodyParser) {
		connection->bodyParser(connection->request, at, length);
	}

	if(connection->resource != nullptr && connection->resource->onBody) {
		return connection->resource->onBody(*connection, connection->request, at, length);
	}

	// TODO: ...
	//	if(connection->response.inputStream != nullptr) {
	//		int res = connection->response.inputStream->write((const uint8_t *)&at, length);
	//		if (res != length) {
	//			connection->response.inputStream->close();
	//			return 1;
	//		}
	//	}

	return 0;
}

err_t HttpServerConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		return TcpConnection::onReceive(buf); // close the connection on TCP error.
	}

	pbuf* cur = buf;
	if(parser.upgrade && resource != nullptr && resource->onUpgrade) {
		while(cur != nullptr && cur->len > 0) {
			int err = resource->onUpgrade(*this, request, (char*)cur->payload, cur->len);
			if(err) {
				debug_e("The upgraded connection returned error: %d", err);
				TcpConnection::onReceive(nullptr);
				return ERR_ABRT; // abort the connection
			}

			cur = cur->next;
		}

		TcpConnection::onReceive(buf);

		return ERR_OK;
	}

	int parsedBytes = 0;
	while(cur != nullptr && cur->len > 0) {
		parsedBytes += http_parser_execute(&parser, &parserSettings, (char*)cur->payload, cur->len);
		if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
			// we ran into trouble - abort the connection
			debug_e("HTTP parser error: %s", httpGetErrorName(HTTP_PARSER_ERRNO(&parser)).c_str());
			sendError();

			if(HTTP_PARSER_ERRNO(&parser) >= HPE_INVALID_EOF_STATE) {
				TcpConnection::onReceive(nullptr);
				return ERR_ABRT; // abort the connection on HTTP parsing error.
			}

			TcpConnection::onReceive(buf);
			return ERR_OK;
		}

		cur = cur->next;
	}

	if(parsedBytes != buf->tot_len) {
		if(!parser.upgrade) {
			// something went wrong
			TcpConnection::onReceive(nullptr);
			return ERR_ABRT; // abort the c
		}

		if(resource != nullptr && resource->onUpgrade) {
			// we have rest bytes -> process them
			while(cur != nullptr && cur->len > 0) {
				int err = resource->onUpgrade(*this, request, (char*)cur->payload, cur->len);
				if(err) {
					debug_e("The upgraded connection returned error: %d", err);
					TcpConnection::onReceive(nullptr);
					return ERR_ABRT; // abort the connection
				}

				cur = cur->next;
			}
		}
	}

	// Fire ReadyToSend callback
	TcpConnection::onReceive(buf);

	return ERR_OK;
}

void HttpServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch(state) {
	case eHCS_StartSending: {
		sendResponseHeaders(&response);
		state = eHCS_SendingHeaders;
	}

	case eHCS_SendingHeaders: {
		if(stream != nullptr && !stream->isFinished()) {
			break;
		}

		state = eHCS_StartBody;
	}

	case eHCS_StartBody:
	case eHCS_SendingBody: {
		if(!sendResponseBody(&response)) {
			break;
		}

		delete stream;
		stream = nullptr;
		state = eHCS_Sent;
	}

	case eHCS_Sent: {
		if(response.headers[HTTP_HEADER_CONNECTION] == _F("close")) {
			setTimeOut(1); // decrease the timeout to 1 tick
		}

		response.reset();
		request.reset();

		state = eHCS_Ready;

		break;
	}

	} /* switch(state) */

	TcpClient::onReadyToSendData(sourceEvent);
}

void HttpServerConnection::sendResponseHeaders(HttpResponse* response)
{
#ifndef DISABLE_HTTPSRV_ETAG
	if(response->stream != nullptr && !response->headers.contains(HTTP_HEADER_ETAG)) {
		String tag = response->stream->id();
		if(tag.length() > 0) {
			response->headers[HTTP_HEADER_ETAG] = String('"' + tag + '"');
		}
	}

	if(request.headers.contains(HTTP_HEADER_IF_MATCH) && response->headers.contains(HTTP_HEADER_ETAG) &&
	   request.headers[HTTP_HEADER_IF_MATCH] == response->headers[HTTP_HEADER_ETAG]) {
		if(request.method == HTTP_GET || request.method == HTTP_HEAD) {
			response->code = HTTP_STATUS_NOT_MODIFIED;
			response->headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
			delete response->stream;
			response->stream = nullptr;
		}
	}
#endif /* DISABLE_HTTPSRV_ETAG */
	String statusLine =
		F("HTTP/1.1 ") + String(response->code) + ' ' + httpGetStatusText((enum http_status)response->code) + "\r\n";
	sendString(statusLine);
	if(response->stream != nullptr && response->stream->available() >= 0) {
		response->headers[HTTP_HEADER_CONTENT_LENGTH] = String(response->stream->available());
	}
	if(!response->headers.contains(HTTP_HEADER_CONTENT_LENGTH) && response->stream == nullptr) {
		response->headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
	}

	if(!response->headers.contains(HTTP_HEADER_CONNECTION)) {
		if(request.headers[HTTP_HEADER_CONNECTION] == _F("close")) {
			// the other side requests closing of the tcp connection...
			response->headers[HTTP_HEADER_CONNECTION] = _F("close");
		} else {
			response->headers[HTTP_HEADER_CONNECTION] = _F("keep-alive"); // Keep-Alive to reuse the connection
		}
	}

#if HTTP_SERVER_EXPOSE_NAME == 1
	response->headers[F("Server")] = _F("HttpServer/Sming");
#endif

#if HTTP_SERVER_EXPOSE_DATE == 1
	response->headers[HTTP_HEADER_DATE] = SystemClock.getSystemTimeString();
#endif
	for(unsigned i = 0; i < response->headers.count(); i++) {
		sendString(response->headers[i]);
	}
	sendString("\r\n");
}

bool HttpServerConnection::sendResponseBody(HttpResponse* response)
{
	if(state == eHCS_StartBody) {
		state = eHCS_SendingBody;
		if(request.method == HTTP_HEAD) {
			if(response->stream != nullptr) {
				delete response->stream;
				response->stream = nullptr;
			}
			return true;
		}

		if(response->stream == nullptr) {
			return true;
		}

		delete stream;
		if(response->headers[HTTP_HEADER_TRANSFER_ENCODING] == _F("chunked")) {
			stream = new ChunkedStream(response->stream);
		} else {
			stream = response->stream; // avoid intermediate buffers
		}
		response->stream = nullptr;

		return false;
	}

	if(stream == nullptr) {
		// we are done for now
		return true;
	}

	if(response->stream == nullptr && !stream->isFinished()) {
		return false;
	}

	return true;
}

void HttpServerConnection::onError(err_t err)
{
	TcpClient::onError(err);
}

void HttpServerConnection::send()
{
	state = eHCS_StartSending;
	onReadyToSendData(eTCE_Received);
}

void HttpServerConnection::sendError(const String& message, enum http_status code)
{
	debug_d("SEND ERROR PAGE");
	response.code = code;
	response.setContentType(MIME_HTML);

	String html = F("<H2 color='#444'>");
	html += message ? message : httpGetStatusText(response.code);
	html += F("</H2>");
	response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
	response.sendString(html);

	send();
}
