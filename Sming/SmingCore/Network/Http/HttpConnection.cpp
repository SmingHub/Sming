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

#include "HttpConnection.h"
#include "../../Data/Stream/ChunkedStream.h"
#include "../../Services/WebHelpers/escape.h"

#ifdef __linux__
#include "lwip/priv/tcp_priv.h"
#else
#include "lwip/tcp_impl.h"
#endif

bool HttpConnection::parserSettingsInitialized = false;
http_parser_settings HttpConnection::parserSettings;

HttpConnection::HttpConnection(RequestQueue* queue): TcpClient(false), mode(eHCM_String)
{
	this->waitingQueue = queue;

	http_parser_init(&parser, HTTP_RESPONSE);
	parser.data = (void*)this;

	if(!parserSettingsInitialized) {
		memset(&parserSettings, 0, sizeof(parserSettings));

		// Notification callbacks: on_message_begin, on_headers_complete, on_message_complete.
		parserSettings.on_message_begin     = staticOnMessageBegin;
		parserSettings.on_headers_complete  = staticOnHeadersComplete;
		parserSettings.on_message_complete  = staticOnMessageComplete;

#ifndef COMPACT_MODE
		parserSettings.on_chunk_header   = staticOnChunkHeader;
		parserSettings.on_chunk_complete = staticOnChunkComplete;
#endif

		// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
#ifndef COMPACT_MODE
		parserSettings.on_status            = staticOnStatus;
#endif
		parserSettings.on_header_field      = staticOnHeaderField;
		parserSettings.on_header_value      = staticOnHeaderValue;
		parserSettings.on_body              = staticOnBody;

		parserSettingsInitialized = true;
	}
}

bool HttpConnection::connect(const String& host, int port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{

	debug_d("HttpConnection::connect: TCP state: %d, isStarted: %d, isActive: %d", (tcp != NULL? tcp->state : -1), (int)(getConnectionState() != eTCS_Ready), (int)isActive());

	if(isProcessing()) {
		return true;
	}

	if(getConnectionState() != eTCS_Ready && isActive()) {
		debug_d("HttpConnection::reusing TCP connection ");

		// we might have still alive connection
		onConnected(ERR_OK);
		return true;
	}

	debug_d("HttpConnection::connecting ...");

	return TcpClient::connect(host, port, useSsl, sslOptions);
}

bool HttpConnection::isActive()
{
	if(tcp == NULL) {
		return false;
	}

	struct tcp_pcb *pcb;
	for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) {
		if(tcp == pcb) {
			return true;
		}
	}

	return false;
}

// @deprecated
HttpHeaders &HttpConnection::getResponseHeaders()
{
	return responseHeaders;
}

String HttpConnection::getResponseHeader(String headerName, String defaultValue /* = "" */)
{
	if (responseHeaders.contains(headerName))
		return responseHeaders[headerName];

	return defaultValue;
}

DateTime HttpConnection::getLastModifiedDate()
{
	DateTime res;
	String strLM = getResponseHeader("Last-Modified");
	if (res.parseHttpDate(strLM))
		return res;
	else
		return DateTime();
}

DateTime HttpConnection::getServerDate()
{
	DateTime res;
	String strSD = getResponseHeader("Date");
	if (res.parseHttpDate(strSD))
		return res;
	else
		return DateTime();
}

String HttpConnection::getResponseString()
{
	if (mode == eHCM_String)
		return responseStringData;
	else
		return "";
}

// @enddeprecated

void HttpConnection::reset()
{
	if(incomingRequest != NULL) {
		delete incomingRequest;
		incomingRequest = NULL;
	}

	code = 0;
	if(responseStringData.length()) {
		responseStringData = "";
	}
	responseHeaders.clear();

	lastWasValue = true;
	lastData = "";
	currentField  = "";
}


err_t HttpConnection::onProtocolUpgrade(http_parser* parser)
{
	debug_w("onProtocolUpgrade: Protocol upgrade is not supported");
	return ERR_ABRT;
}

int HttpConnection::staticOnMessageBegin(http_parser* parser)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	connection->reset();

	connection->incomingRequest = connection->executionQueue.dequeue();
	if(connection->incomingRequest == NULL) {
		return 1; // there are no requests in the queue
	}

	if(connection->incomingRequest->responseStream != NULL) {
		connection->mode = eHCM_Stream;
	}
	else {
		connection->mode = eHCM_String;
	}

	return 0;
}

int HttpConnection::staticOnMessageComplete(http_parser* parser)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
	if(connection == NULL) {
		// something went wrong
		return -1;
	}

	if(!connection->incomingRequest) {
		return -2; // no current request...
	}

	debug_d("staticOnMessageComplete: Execution queue: %d, %s",
								connection->executionQueue.count(),
								connection->incomingRequest->uri.toString().c_str()
								);

	// we are finished with this request
	int hasError = 0;
	if(connection->incomingRequest->requestCompletedDelegate) {
		bool success = (HTTP_PARSER_ERRNO(parser) == HPE_OK) &&  // false when the parsing has failed
					   (connection->code >= 200 && connection->code <= 399);  // false when the HTTP status code is not ok
		hasError = connection->incomingRequest->requestCompletedDelegate(*connection, success);
	}

	if(connection->incomingRequest->auth != NULL) {
		connection->incomingRequest->auth->setResponse(connection->getResponse());
	}

	if(connection->incomingRequest->retries > 0) {
		connection->incomingRequest->retries--;
		return (connection->executionQueue.enqueue(connection->incomingRequest)? 0: -1);
	}

	if(connection->incomingRequest->responseStream != NULL) {
		connection->incomingRequest->responseStream->close();
		delete connection->incomingRequest->responseStream;
		connection->incomingRequest->responseStream = NULL;
	}

	delete connection->incomingRequest;
	connection->incomingRequest = NULL;

	if(!connection->executionQueue.count()) {
		connection->onConnected(ERR_OK);
	}

	return hasError;
}

int HttpConnection::staticOnHeadersComplete(http_parser* parser)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
	if(connection == NULL) {
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
	 * expect neither a body nor any futher responses on this connection. This is
	 * useful for handling responses to a CONNECT request which may not contain
	 * `Upgrade` or `Connection: upgrade` headers.
	 */

	connection->code = parser->status_code;
	if(connection->incomingRequest == NULL) {
		// nothing to process right now...
		return 1;
	}

	int error = 0;
	if(connection->incomingRequest->headersCompletedDelegate) {
		error = connection->incomingRequest->headersCompletedDelegate(*connection, connection->responseHeaders);
	}

	if(!error && connection->incomingRequest->method == HTTP_HEAD) {
		error = 1;
	}

	return error;
}

#ifndef COMPACT_MODE
int HttpConnection::staticOnStatus(http_parser *parser, const char *at, size_t length)
{
	return 0;
}
#endif

int HttpConnection::staticOnHeaderField(http_parser *parser, const char *at, size_t length)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
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

int HttpConnection::staticOnHeaderValue(http_parser *parser, const char *at, size_t length)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
	if (connection == NULL) {
		// something went wrong
		return -1;
	}

	if(!connection->lastWasValue) {
		connection->currentField = connection->lastData;
		connection->responseHeaders[connection->currentField] = "";
		connection->lastWasValue = true;
	}
	connection->responseHeaders[connection->currentField] += String(at, length);

	return 0;
}

int HttpConnection::staticOnBody(http_parser *parser, const char *at, size_t length)
{
	HttpConnection *connection = (HttpConnection*)parser->data;
	if (connection == NULL) {
		// something went wrong
		return -1;
	}

	if(connection->incomingRequest->requestBodyDelegate) {
		return connection->incomingRequest->requestBodyDelegate(*connection, at, length);
	}

	if (connection->mode == eHCM_String) {
		connection->responseStringData += String(at, length);
		return 0;
	}

	if(connection->incomingRequest->responseStream != NULL) {
		int res = connection->incomingRequest->responseStream->write((const uint8_t *)at, length);
		if (res != length) {
			connection->incomingRequest->responseStream->close();
			return 1;
		}
	}

	return 0;
}

#ifndef COMPACT_MODE
int HttpConnection::staticOnChunkHeader(http_parser* parser)
{
	debug_d("On chunk header");
	return 0;
}

int HttpConnection::staticOnChunkComplete(http_parser* parser)
{
	debug_d("On chunk complete");
	return 0;
}
#endif

void HttpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{

	debug_d("HttpConnection::onReadyToSendData: waitingQueue.count: %d", waitingQueue->count());

	do {
		if(state == eHCS_Sent) {
			state = eHCS_Ready;
		}

		if(state == eHCS_Ready) {
			HttpRequest* request = waitingQueue->peek();
			if(request == NULL) {
				debug_d("Nothing in the waiting queue");
				outgoingRequest = NULL;
				break;
			}

			// if the executionQueue is not empty then we have to check if we can pipeline that request
			if(executionQueue.count()) {
				if(!(request->method == HTTP_GET || request->method == HTTP_HEAD)) {
					// if the current request cannot be pipelined -> break;
					break;
				}

				// if we have previous request
				if(outgoingRequest != NULL) {
					if(!(outgoingRequest->method == HTTP_GET || outgoingRequest->method == HTTP_HEAD)) {
						// the outgoing request does not allow pipelining
						break;
					}
				}
			} // executionQueue.count()

			if(!executionQueue.enqueue(request)) {
				debug_e("The working queue is full at the moment");
				break;
			}

			waitingQueue->dequeue();

			outgoingRequest = request;
			state = eHCS_SendingHeaders;
			sendRequestHeaders(request);

			break;
		}

		if(state >= eHCS_StartSending && state < eHCS_Sent) {
			if(state == eHCS_SendingHeaders) {
				if(stream != NULL && !stream->isFinished()) {
						break;
				}

				state = eHCS_StartBody;
			}

			if(sendRequestBody(outgoingRequest)) {
				state = eHCS_Sent;
				delete stream;
				stream = NULL;
				continue;
			}
		}

		break;

	} while(true);

	TcpClient::onReadyToSendData(sourceEvent);
}

void HttpConnection::sendRequestHeaders(HttpRequest* request)
{
	sendString(http_method_str(request->method) + String(" ") + request->uri.getPathWithQuery() + " HTTP/1.1\r\nHost: " + request->uri.Host + "\r\n");

	// TODO: represent the post params as stream ...

	// Adjust the content-length
	request->headers["Content-Length"] = "0";
	if(request->rawDataLength) {
		request->headers["Content-Length"] = String(request->rawDataLength);
	}
	else if (request->stream != NULL) {
		if(request->stream->available() > -1) {
			request->headers["Content-Length"] = String(request->stream->available());
		}
		else {
			request->headers.remove("Content-Length");
		}
	}

	if(!request->headers.contains("Content-Length")) {
		request->headers["Transfer-Encoding"] = "chunked";
	}

	if(request->postParams.count() && !request->headers.contains("Content-Type")) {
		request->headers["Content-Type"] = ContentType::toString(MIME_FORM_URL_ENCODED);
	}

	for (int i = 0; i < request->headers.count(); i++)
	{
		String write = request->headers.keyAt(i) + ": " + request->headers.valueAt(i) + "\r\n";
		sendString(write.c_str());
	}
	sendString("\r\n");
}

bool HttpConnection::sendRequestBody(HttpRequest* request)
{
	if(state == eHCS_StartBody) {
		state = eHCS_SendingBody;
		// if there is input raw data -> send it
		if(request->rawDataLength > 0) {
			TcpClient::send((const char*)request->rawData, (uint16_t)request->rawDataLength);
			request->rawDataLength = 0;

			return false;
		}

#if 0
		// Post Params should be also stream...
		if (request->postParams.count())  {
			for(int i = 0; i < request->postParams.count(); i++) {
				// TODO: prevent memory fragmentation ...
				char *dest = uri_escape(NULL, 0, request->postParams.valueAt(i).c_str(), request->postParams.valueAt(i).length());
				String write = request->postParams.keyAt(i) + "=" + String(dest) + "&";
				sendString(write.c_str());
				free(dest);
			}
		}
#endif

		if(request->stream == NULL) {
			return true;
		}

		delete stream;
		if(request->headers["Transfer-Encoding"] == "chunked") {
			stream = new ChunkedStream(request->stream);
		}
		else {
			stream = request->stream; // avoid intermediate buffers
		}
		request->stream = NULL;
		return false;
	}

	if(stream == NULL) {
		// we are done for now
		return true;
	}

	if(request->stream == NULL && !stream->isFinished()) {
		return false;
	}

	return true;
}

HttpRequest* HttpConnection::getRequest()
{
	return incomingRequest;
}

HttpResponse* HttpConnection::getResponse()
{
	HttpResponse* response = new HttpResponse();
	response->code = code;
	response->headers = responseHeaders;
// TODO: fix this...
//	if(currentRequest) {
//		response->stream = currentRequest->outputStream;
//	}

	if(responseStringData.length()) {
		if(response->stream != NULL) {
			delete response->stream;
			response->stream = NULL;
		}

		MemoryDataStream* memory = new MemoryDataStream();
		memory->write((uint8_t *)responseStringData.c_str(), responseStringData.length());
		response->stream = memory;
	}
	return response;
}

// end of public methods for HttpConnection

err_t HttpConnection::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	pbuf *cur = buf;
	int parsedBytes = 0;
	while (cur != NULL && cur->len > 0) {
		parsedBytes += http_parser_execute(&parser, &parserSettings, (char*) cur->payload, cur->len);
		if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
			// we ran into trouble - abort the connection
			debug_e("HTTP parser error: %s", http_errno_name(HTTP_PARSER_ERRNO(&parser)));
			cleanup();
			TcpConnection::onReceive(NULL);
			return ERR_ABRT;
		}

		cur = cur->next;
	}

	if (parser.upgrade) {
		return onProtocolUpgrade(&parser);
	} else if (parsedBytes != buf->tot_len) {
		TcpClient::onReceive(NULL);

		return ERR_ABRT;
	}

	// Fire ReadyToSend callback
	TcpClient::onReceive(buf);

	return ERR_OK;
}

void HttpConnection::onError(err_t err)
{
	cleanup();
	TcpClient::onError(err);
}

void HttpConnection::cleanup()
{
	// TODO: clean the current request
	reset();

	// TODO: clean the current response

	// if there are requests in the executionQueue -> move them back to the waiting queue
	for(int i=0; i < executionQueue.count(); i++) {
		waitingQueue->enqueue(executionQueue.dequeue());
	}
}

HttpConnection::~HttpConnection()
{
	cleanup();
}

