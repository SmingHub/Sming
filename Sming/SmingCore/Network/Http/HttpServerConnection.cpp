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
#include "../../Services/cWebsocket/websocket.h"
#include "WebConstants.h"

HttpServerConnection::HttpServerConnection(tcp_pcb *clientTcp)
	: TcpClient(clientTcp, 0, 0), state(eHCS_Ready)
{
	// create parser ...
	http_parser_init(&parser, HTTP_REQUEST);
	parser.data = (void*)this;

	memset(&parserSettings, 0, sizeof(parserSettings));
	// Notification callbacks: on_message_begin, on_headers_complete, on_message_complete.
	parserSettings.on_message_begin     = staticOnMessageBegin;
	parserSettings.on_headers_complete  = staticOnHeadersComplete;
	parserSettings.on_message_complete  = staticOnMessageComplete;

	// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
	parserSettings.on_url               = staticOnPath;
	parserSettings.on_header_field      = staticOnHeaderField;
	parserSettings.on_header_value      = staticOnHeaderValue;
	parserSettings.on_body              = staticOnBody;
}

HttpServerConnection::~HttpServerConnection()
{
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
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	// Reset Response ...
	connection->response.code = 200;
	connection->response.headers.clear();
	if(connection->response.stream != NULL) {
		delete connection->response.stream;
		connection->response.stream = NULL;
	}

	connection->headersSent = false;
	connection->state = eHCS_Ready;

	// ... and Request
	// TODO:
	connection->request.setMethod((const HttpMethod)parser->method);

	// and temp data...
	connection->requestHeaders.clear();
	connection->bodyParser = 0;

	return 0;
}

int HttpServerConnection::staticOnPath(http_parser *parser, const char *at, size_t length) {
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	// TODO: find the most suitable path..

	String path = String(at, length);
	if (path.length() > 1 && path.endsWith("/")) {
		path = path.substring(0, path.length() - 1);
	}

	connection->request.setURL(path);

	if(connection->resourceTree == NULL) {
		debugf("WARNING: HttpServerConnection: The resource tree is not set!");

		return -1;
	}

	if (connection->resourceTree->contains(connection->request.uri.Path)) {
		connection->resource = (*connection->resourceTree)[connection->request.uri.Path];
	}
	else if(connection->resourceTree->contains("*")) {
		connection->resource = (*connection->resourceTree)["*"];
	}

	return 0;
}

int HttpServerConnection::staticOnMessageComplete(http_parser* parser)
{
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	// we are finished with this request
	int hasError = 0;
	if(HTTP_PARSER_ERRNO(parser) != HPE_OK) {
		connection->sendError(http_errno_name(HTTP_PARSER_ERRNO(parser)));
		return 0;
	}

	if(connection->bodyParser) {
		connection->bodyParser(connection->request, NULL, -2);
	}

	if(connection->resource != NULL && connection->resource->onRequestComplete) {
		hasError = connection->resource->onRequestComplete(*connection, connection->request, connection->response);
	}

	connection->send();

	if(connection->request.responseStream != NULL) {
		connection->request.responseStream->close();
		delete connection->request.responseStream;
	}

	return hasError;
}

int HttpServerConnection::staticOnHeadersComplete(http_parser* parser)
{
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	debugf("The headers are complete");

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
	 * expect neither a body nor any futher responses on this connection. This is
	 * useful for handling responses to a CONNECT request which may not contain
	 * `Upgrade` or `Connection: upgrade` headers.
	 */
	int error = 0;
	connection->request.setHeaders(connection->requestHeaders);

	if(connection->resource != NULL && connection->resource->onHeadersComplete) {
		error = connection->resource->onHeadersComplete(*connection, connection->request, connection->response);
	}

	if(!error && connection->request.method == HTTP_HEAD) {
		error = 1;
	}

	if(connection->request.headers.contains("Content-Type")) {
		String contentType = connection->request.headers["Content-Type"];
		int endPos = contentType.indexOf(';');
		if(endPos != -1) {
			contentType = contentType.substring(0, endPos);
		}

		if(connection->bodyParsers->contains(contentType)) {
			connection->bodyParser = (*connection->bodyParsers)[contentType];
			connection->bodyParser(connection->request, NULL, -1);
		}
	}

	return error;
}

int HttpServerConnection::staticOnHeaderField(http_parser *parser, const char *at, size_t length)
{
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if(connection == NULL) {
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

int HttpServerConnection::staticOnHeaderValue(http_parser *parser, const char *at, size_t length)
{
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if (connection == NULL) {
		// something went wrong
		return -1;
	}

	if(!connection->lastWasValue) {
		connection->currentField = connection->lastData;
		connection->requestHeaders[connection->currentField] = "";
		connection->lastWasValue = true;
	}
	connection->requestHeaders[connection->currentField] += String(at, length);

	return 0;
}

int HttpServerConnection::staticOnBody(http_parser *parser, const char *at, size_t length)
{
	HttpServerConnection *connection = (HttpServerConnection*)parser->data;
	if (connection == NULL) {
		// something went wrong
		return -1;
	}

	if(connection->bodyParser) {
		connection->bodyParser(connection->request, at, length);
	}

	if(connection->resource != NULL && connection->resource->onBody) {
		return connection->resource->onBody(*connection, connection->request, at, length);
	}

	// TODO: ...
//	if(connection->response.inputStream != NULL) {
//		int res = connection->response.inputStream->write((const uint8_t *)&at, length);
//		if (res != length) {
//			connection->response.inputStream->close();
//			return 1;
//		}
//	}

	return 0;
}

err_t HttpServerConnection::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		return TcpConnection::onReceive(buf); // close the connection on TCP error.
	}

	pbuf *cur = buf;
	if (parser.upgrade && resource != NULL && resource->onUpgrade) {
		while (cur != NULL && cur->len > 0) {
			int err = resource->onUpgrade(*this, request, (char*)cur->payload, cur->len);
			if(err) {
				debugf("The upgraded connection returned error: %d", err);
				TcpConnection::onReceive(NULL);
				return ERR_ABRT; // abort the connection
			}

			cur = cur->next;
		}

		TcpConnection::onReceive(buf);

		return ERR_OK;
	}

	int parsedBytes = 0;
	while (cur != NULL && cur->len > 0) {
		parsedBytes += http_parser_execute(&parser, &parserSettings, (char*) cur->payload, cur->len);
		if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
			// we ran into trouble - abort the connection
			debugf("HTTP parser error: %s", http_errno_name(HTTP_PARSER_ERRNO(&parser)));
			sendError();

			if(HTTP_PARSER_ERRNO(&parser) >= HPE_INVALID_EOF_STATE) {
				TcpConnection::onReceive(NULL);
				return ERR_ABRT; // abort the connection on HTTP parsing error.
			}

			TcpConnection::onReceive(buf);
			return ERR_OK;
		}

		cur = cur->next;
	}

	if (parsedBytes != buf->tot_len) {
		if(!parser.upgrade) {
			// something went wrong
			TcpConnection::onReceive(NULL);
			return ERR_ABRT; // abort the c
		}

		if(resource != NULL && resource->onUpgrade) {
			// we have rest bytes -> process them
			while (cur != NULL && cur->len > 0) {
				int err = resource->onUpgrade(*this, request, (char*)cur->payload, cur->len);
				if(err) {
					debugf("The upgraded connection returned error: %d", err);
					TcpConnection::onReceive(NULL);
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
	if(state != eHCS_Sending) {
		TcpClient::onReadyToSendData(sourceEvent);
		return;
	}

	if(!headersSent) {
		String statusLine = "HTTP/1.1 "+String(response.code) +  " " + getStatus((enum http_status)response.code) + "\r\n";
		writeString(statusLine, TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);

		if(response.stream != NULL && response.stream->length() != -1) {
			response.headers["Content-Length"] = String(response.stream->length());
		}
		if(!response.headers.contains("Content-Length") && response.stream == NULL) {
			response.headers["Content-Length"] = "0";
		}

		if(!response.headers.contains("Connection")) {
			if(request.headers.contains("Connection") && request.headers["Connection"] == "close") {
				// the other side requests closing of the tcp connection...
				response.headers["Connection"] = "close";
			}
			else {
				response.headers["Connection"] = "keep-alive"; // Keep-Alive to reuse the connection
			}
		}

#if HTTP_SERVER_EXPOSE_NAME == 1
		response.headers["Server"] = "HttpServer/Sming";
#endif

#if HTTP_SERVER_EXPOSE_DATE == 1
		response.headers["Date"] = SystemClock.getSystemTimeString();
#endif
		for (int i = 0; i < response.headers.count(); i++)
		{
			String write = response.headers.keyAt(i) + ": " + response.headers.valueAt(i) + "\r\n";
			writeString(write.c_str(), TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
		}
		writeString("\r\n", TCP_WRITE_FLAG_MORE | TCP_WRITE_FLAG_COPY);
		headersSent = true;
	}

	do {
		if(request.method == HTTP_HEAD) {
			if(response.stream != NULL) {
				delete response.stream;
				response.stream = NULL;
			}
			state = eHCS_Sent;
			break;
		}

		if(response.stream == NULL) {
			state = eHCS_Sent;
			break;
		}

		write(response.stream);
		if (response.stream->isFinished()) {
			debugf("Body stream completed");
			delete response.stream; // Free memory now!
			response.stream = NULL;
			state = eHCS_Sent;
		}
	} while(false);

	if(state == eHCS_Sent && response.headers["Connection"] == "close") {
		setTimeOut(1); // decrease the timeout to 1 tick
	}

	TcpClient::onReadyToSendData(sourceEvent);
}

void HttpServerConnection::onError(err_t err) {
	TcpClient::onError(err);
}

const char * HttpServerConnection::getStatus(enum http_status code)
{
  switch (code) {
#define XX(num, name, string) case num : return #string;
      HTTP_STATUS_MAP(XX)
#undef XX
      default: return "<unknown>";
  }
}

void HttpServerConnection::send()
{
	state = eHCS_Sending;
}

void HttpServerConnection::sendError(const char* message /* = NULL*/, enum http_status code /* = HTTP_STATUS_BAD_REQUEST */)
{
	debugf("SEND ERROR PAGE");
	response.code = code;
	response.setContentType(MIME_HTML);

	String html = "<H2 color='#444'>";
	html += message ? message :  getStatus((enum http_status)response.code);
	html += "</H2>";
	response.headers["Content-Length"] = html.length();
	response.sendString(html);

	send();
}
