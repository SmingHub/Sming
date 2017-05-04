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

#include "../../Services/WebHelpers/escape.h"

#ifdef __linux__
#include "lwip/priv/tcp_priv.h"
#else
#include "lwip/tcp_impl.h"
#endif

HttpConnection::HttpConnection(RequestQueue* queue): TcpClient(false), mode(eHCM_String) {
	this->waitingQueue = queue;
}

bool HttpConnection::connect(const String& host, int port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */) {

	debugf("HttpConnection::connect: TCP state: %d, isStarted: %d, isActive: %d", (tcp != NULL? tcp->state : -1), (int)(getConnectionState() != eTCS_Ready), (int)isActive());

	if(isProcessing()) {
		return true;
	}

	if(getConnectionState() != eTCS_Ready && isActive()) {
		debugf("HttpConnection::reusing TCP connection ");

		// we might have still alive connection
		onConnected(ERR_OK);
		return true;
	}

	debugf("HttpConnection::connecting ...");

	return TcpClient::connect(host, port, useSsl, sslOptions);
}

bool HttpConnection::isActive() {
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
HashMap<String, String> &HttpConnection::getResponseHeaders()
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
	if(currentRequest != NULL) {
		delete currentRequest;
		currentRequest = NULL;
	}

	code = 0;
	responseStringData = "";
	responseHeaders.clear();

	lastWasValue = true;
	lastData = "";
	currentField  = "";
}


err_t HttpConnection::onProtocolUpgrade(http_parser* parser)
{
	debugf("onProtocolUpgrade: Protocol upgrade is not supported");
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

	connection->currentRequest = connection->executionQueue.dequeue();
	if(connection->currentRequest == NULL) {
		return 1; // there are no requests in the queue
	}

	if(connection->currentRequest->responseStream != NULL) {
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

	if(!connection->currentRequest) {
		return -2; // no current request...
	}

	debugf("staticOnMessageComplete: Execution queue: %d, %s",
								connection->executionQueue.count(),
								connection->currentRequest->uri.toString().c_str()
								);

	// we are finished with this request
	int hasError = 0;
	if(connection->currentRequest->requestCompletedDelegate) {
		bool success = (HTTP_PARSER_ERRNO(parser) == HPE_OK) &&  // false when the parsing has failed
					   (connection->code >= 200 && connection->code <= 399);  // false when the HTTP status code is not ok
		hasError = connection->currentRequest->requestCompletedDelegate(*connection, success);
	}

	if(connection->currentRequest->auth != NULL) {
		connection->currentRequest->auth->setResponse(connection->getResponse());
	}

	if(connection->currentRequest->retries > 0) {
		connection->currentRequest->retries--;
		return (connection->executionQueue.enqueue(connection->currentRequest)? 0: -1);
	}

	if(connection->currentRequest->responseStream != NULL) {
		connection->currentRequest->responseStream->close();
		delete connection->currentRequest->responseStream;
	}

	delete connection->currentRequest;
	connection->currentRequest = NULL;

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

	connection->code = parser->status_code;
	if(connection->currentRequest == NULL) {
		// nothing to process right now...
		return 1;
	}

	int error = 0;
	if(connection->currentRequest->headersCompletedDelegate) {
		error = connection->currentRequest->headersCompletedDelegate(*connection, connection->responseHeaders);
	}

	if(!error && connection->currentRequest->method == HTTP_HEAD) {
		error = 1;
	}

	return error;
}

int HttpConnection::staticOnStatus(http_parser *parser, const char *at, size_t length) {
	return 0;
}

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

	if(connection->currentRequest->requestBodyDelegate) {
		return connection->currentRequest->requestBodyDelegate(*connection, at, length);
	}

	if (connection->mode == eHCM_String) {
		connection->responseStringData += String(at, length);
		return 0;
	}

	if(connection->currentRequest->responseStream != NULL) {
		int res = connection->currentRequest->responseStream->write((const uint8_t *)at, length);
		if (res != length) {
			connection->currentRequest->responseStream->close();
			return 1;
		}
	}

	return 0;
}

int HttpConnection::staticOnChunkHeader(http_parser* parser) {
	debugf("On chunk header");
	return 0;
}

int HttpConnection::staticOnChunkComplete(http_parser* parser) {
	debugf("On chunk complete");
	return 0;
}

err_t HttpConnection::onConnected(err_t err) {
	if (err == ERR_OK) {
		// create parser ...
		if(parser == NULL) {
			parser = new http_parser;
			http_parser_init(parser, HTTP_RESPONSE);
			parser->data = (void*)this;

			memset(&parserSettings, 0, sizeof(parserSettings));
			// Notification callbacks: on_message_begin, on_headers_complete, on_message_complete.
			parserSettings.on_message_begin     = staticOnMessageBegin;
			parserSettings.on_headers_complete  = staticOnHeadersComplete;
			parserSettings.on_message_complete  = staticOnMessageComplete;

			parserSettings.on_chunk_header   = staticOnChunkHeader;
			parserSettings.on_chunk_complete = staticOnChunkComplete;


			// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
			parserSettings.on_status            = staticOnStatus;
			parserSettings.on_header_field      = staticOnHeaderField;
			parserSettings.on_header_value      = staticOnHeaderValue;
			parserSettings.on_body              = staticOnBody;
		}

		debugf("HttpConnection::onConnected: waitingQueue.count: %d", waitingQueue->count());

		do {
			HttpRequest* request = waitingQueue->peek();
			if(request == NULL) {
				break;
			}

			if(!executionQueue.enqueue(request)) {
				debugf("The working queue is full at the moment");
				break;
			}

			waitingQueue->dequeue();
			send(request);

			if(!(request->method == HTTP_GET || request->method == HTTP_HEAD)) {
				// if the current request cannot be pipelined -> break;
				break;
			}

			HttpRequest* nextRequest = waitingQueue->peek();
			if(nextRequest != NULL && !(nextRequest->method == HTTP_GET || nextRequest->method == HTTP_HEAD))  {
				// if the next request cannot be pipelined -> break for now
				break;
			}
		} while(1);
	}

	TcpClient::onConnected(err);
	return ERR_OK;
}

void HttpConnection::send(HttpRequest* request) {
	sendString(http_method_str(request->method) + String(" ") + request->uri.getPathWithQuery() + " HTTP/1.1\r\nHost: " + request->uri.Host + "\r\n");

	// Adjust the content-length
	request->headers["Content-Length"] = "0";
	if(request->rawDataLength) {
		request->headers["Content-Length"] = String(request->rawDataLength);
	}
	else if (request->stream != NULL && request->stream->length() > -1) {
		request->headers["Content-Length"] = String(request->stream->length());
	}

	// TODO: represent the post params as stream ...


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

	// Send content

	// if there is input raw data -> send it
	if(request->rawDataLength > 0) {
		TcpClient::send((const char*)request->rawData, (uint16_t)request->rawDataLength);
	}
	else if(request->stream != NULL) {
		send(request->stream);

		debugf("Stream completed");
		delete request->stream;
		request->stream = NULL;
	}
#if 0

	// Post Params should be also stream...

	else if (request->postParams.count())  {
		for(int i = 0; i < request->postParams.count(); i++) {
			// TODO: prevent memory fragmentation ...
			char *dest = uri_escape(NULL, 0, request->postParams.valueAt(i).c_str(), request->postParams.valueAt(i).length());
			String write = request->postParams.keyAt(i) + "=" + String(dest) + "&";
			sendString(write.c_str());
			free(dest);
		}
	}
#endif
}

bool HttpConnection::send(IDataSourceStream* inputStream, bool forceCloseAfterSent /* = false*/)
{
	if(inputStream->length() != -1) {
		// send the data as one big blob
		do {
			int len = 256;
			char data[len];
			len = inputStream->readMemoryBlock(data, len);
			TcpClient::send(data, len);
			inputStream->seek(max(len, 0));
		} while(!inputStream->isFinished());

		return true;
	}

	// Send the data in chunked-encoding

	do {
		int len = 256;
		char data[len];
		len = inputStream->readMemoryBlock(data, len);

		// send the data in chunks...
		sendString(String(len)+ "\r\n");
		TcpClient::send(data, len);
		sendString("\n\r");
		inputStream->seek(max(len, 0));
	} while(!inputStream->isFinished());

	sendString("0\r\n\r\n", forceCloseAfterSent);

	return true;
}

HttpRequest* HttpConnection::getRequest() {
	return currentRequest;
}

HttpResponse* HttpConnection::getResponse() {
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
		response->stream = (IDataSourceStream* )memory;
	}
	return response;
}

// end of public methods for HttpConnection

err_t HttpConnection::onReceive(pbuf *buf) {
	if (buf == NULL)
	{
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	pbuf *cur = buf;
	int parsedBytes = 0;
	while (cur != NULL && cur->len > 0) {
		parsedBytes += http_parser_execute(parser, &parserSettings, (char*) cur->payload, cur->len);
		if(HTTP_PARSER_ERRNO(parser) != HPE_OK) {
			// we ran into trouble - abort the connection
			debugf("HTTP parser error: %s", http_errno_name(HTTP_PARSER_ERRNO(parser)));
			cleanup();
			TcpConnection::onReceive(NULL);
			return ERR_ABRT;
		}

		cur = cur->next;
	}

	if (parser->upgrade) {
		return onProtocolUpgrade(parser);
	} else if (parsedBytes != buf->tot_len) {
		TcpClient::onReceive(NULL);

		return ERR_ABRT;
	}

	// Fire ReadyToSend callback
	TcpClient::onReceive(buf);

	return ERR_OK;
}

void HttpConnection::onError(err_t err) {
	cleanup();
	TcpClient::onError(err);
}

void HttpConnection::cleanup() {
	// TODO: clean the current request
	reset();

	// TODO: clean the current response

	// if there are requests in the executionQueue -> move them back to the waiting queue
	for(int i=0; i < executionQueue.count(); i++) {
		waitingQueue->enqueue(executionQueue.dequeue());
	}

	if(parser != NULL) {
		delete parser;
		parser = NULL;
	}
}

HttpConnection::~HttpConnection() {
	cleanup();
}

