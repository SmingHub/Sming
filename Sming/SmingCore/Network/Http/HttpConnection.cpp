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
#include "Data/Stream/FileStream.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/LimitedMemoryStream.h"
#include "Data/Stream/ChunkedStream.h"
#include "Data/Stream/UrlencodedOutputStream.h"

#ifdef __linux__
#include "lwip/priv/tcp_priv.h"
#else
#include "lwip/tcp_impl.h"
#endif

bool HttpConnection::parserSettingsInitialized = false;
http_parser_settings HttpConnection::parserSettings;

HttpConnection::HttpConnection(RequestQueue* queue) : TcpClient(false)
{
	this->waitingQueue = queue;

	http_parser_init(&parser, HTTP_RESPONSE);
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

		// Data callbacks: on_url, (common) on_header_field, on_header_value, on_body;
#ifndef COMPACT_MODE
		parserSettings.on_status = staticOnStatus;
#endif
		parserSettings.on_header_field = staticOnHeaderField;
		parserSettings.on_header_value = staticOnHeaderValue;
		parserSettings.on_body = staticOnBody;

		parserSettingsInitialized = true;
	}
}

bool HttpConnection::connect(const String& host, int port, bool useSsl /* = false */, uint32_t sslOptions /* = 0 */)
{
	debug_d("HttpConnection::connect: TCP state: %d, isStarted: %d, isActive: %d", (tcp != nullptr ? tcp->state : -1),
			(int)(getConnectionState() != eTCS_Ready), (int)isActive());

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

bool HttpConnection::send(HttpRequest* request)
{
	return waitingQueue->enqueue(request);
}

bool HttpConnection::isActive()
{
	if(tcp == nullptr) {
		return false;
	}

	struct tcp_pcb* pcb;
	for(pcb = tcp_active_pcbs; pcb != nullptr; pcb = pcb->next) {
		if(tcp == pcb) {
			return true;
		}
	}

	return false;
}

// @deprecated
HttpHeaders& HttpConnection::getResponseHeaders()
{
	return response.headers;
}

String HttpConnection::getResponseHeader(String headerName, String defaultValue)
{
	if(response.headers.contains(headerName))
		return response.headers[headerName];

	return defaultValue;
}

DateTime HttpConnection::getLastModifiedDate()
{
	DateTime res;
	String strLM = response.headers[HTTP_HEADER_LAST_MODIFIED];
	if(res.parseHttpDate(strLM))
		return res;
	else
		return DateTime();
}

DateTime HttpConnection::getServerDate()
{
	DateTime res;
	String strSD = response.headers[HTTP_HEADER_DATE];
	if(res.parseHttpDate(strSD))
		return res;
	else
		return DateTime();
}

String HttpConnection::getResponseString()
{
	return response.getBody();
}

// @enddeprecated

void HttpConnection::reset()
{
	if(incomingRequest != nullptr) {
		delete incomingRequest;
		incomingRequest = nullptr;
	}

	response.reset();

	lastWasValue = true;
	lastData = "";
	currentField = "";
}

err_t HttpConnection::onProtocolUpgrade(http_parser* parser)
{
	debug_w("onProtocolUpgrade: Protocol upgrade is not supported");
	return ERR_ABRT;
}

int HttpConnection::staticOnMessageBegin(http_parser* parser)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	connection->reset();

	connection->incomingRequest = connection->executionQueue.dequeue();
	if(connection->incomingRequest == nullptr) {
		return 1; // there are no requests in the queue
	}

	return 0;
}

HttpPartResult HttpConnection::multipartProducer()
{
	HttpPartResult result;

	if(outgoingRequest->files.count()) {
		String name = outgoingRequest->files.keyAt(0);
		FileStream* file = outgoingRequest->files[name];
		result.stream = file;

		HttpHeaders* headers = new HttpHeaders();
		(*headers)[HTTP_HEADER_CONTENT_DISPOSITION] =
			F("form-data; name=\"") + name + F("\"; filename=\"") + file->fileName() + '"';
		(*headers)[HTTP_HEADER_CONTENT_TYPE] = ContentType::fromFullFileName(file->fileName());
		result.headers = headers;

		outgoingRequest->files.remove(name);
		return result;
	}

	if(outgoingRequest->postParams.count()) {
		String name = outgoingRequest->postParams.keyAt(0);
		String value = outgoingRequest->postParams[name];

		MemoryDataStream* mStream = new MemoryDataStream();
		mStream->write((uint8_t*)value.c_str(), value.length());
		result.stream = mStream;

		HttpHeaders* headers = new HttpHeaders();
		(*headers)[HTTP_HEADER_CONTENT_DISPOSITION] = F("form-data; name=\"") + name + '"';
		result.headers = headers;

		outgoingRequest->postParams.remove(name);
		return result;
	}

	return result;
}

int HttpConnection::staticOnMessageComplete(http_parser* parser)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(!connection->incomingRequest) {
		return -2; // no current request...
	}

	debug_d("staticOnMessageComplete: Execution queue: %d, %s", connection->executionQueue.count(),
			connection->incomingRequest->uri.toString().c_str());

	// we are finished with this request
	int hasError = 0;
	if(connection->incomingRequest->requestCompletedDelegate) {
		bool success = (HTTP_PARSER_ERRNO(parser) == HPE_OK) && // false when the parsing has failed
					   (connection->response.code >= 200 &&
						connection->response.code <= 399); // false when the HTTP status code is not ok
		hasError = connection->incomingRequest->requestCompletedDelegate(*connection, success);
	}

	if(connection->incomingRequest->retries > 0) {
		connection->incomingRequest->retries--;
		return (connection->executionQueue.enqueue(connection->incomingRequest) ? 0 : -1);
	}

	delete connection->incomingRequest;
	connection->incomingRequest = nullptr;

	if(!connection->executionQueue.count()) {
		connection->onConnected(ERR_OK);
	}

	return hasError;
}

int HttpConnection::staticOnHeadersComplete(http_parser* parser)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
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

	if(connection->incomingRequest == nullptr) {
		// nothing to process right now...
		return 1;
	}

	connection->response.code = parser->status_code;

	if(connection->incomingRequest->auth != nullptr) {
		connection->incomingRequest->auth->setResponse(connection->getResponse());
	}

	int error = 0;
	if(connection->incomingRequest->headersCompletedDelegate) {
		error = connection->incomingRequest->headersCompletedDelegate(*connection, connection->response);
	}

	if(!error && connection->incomingRequest->method == HTTP_HEAD) {
		error = 1;
	}

	if(!error) {
		// set the response stream
		if(connection->incomingRequest->responseStream != nullptr) {
			connection->response.stream = connection->incomingRequest->responseStream;
			connection->incomingRequest->responseStream = nullptr; // the response object will release that stream
		} else {
			connection->response.stream = new LimitedMemoryStream(NETWORK_SEND_BUFFER_SIZE);
		}
	}

	return error;
}

#ifndef COMPACT_MODE
int HttpConnection::staticOnStatus(http_parser* parser, const char* at, size_t length)
{
	return 0;
}
#endif

int HttpConnection::staticOnHeaderField(http_parser* parser, const char* at, size_t length)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
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

int HttpConnection::staticOnHeaderValue(http_parser* parser, const char* at, size_t length)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(!connection->lastWasValue) {
		connection->currentField = connection->lastData;
		connection->response.headers[connection->currentField] = nullptr;
		connection->lastWasValue = true;
	}
	connection->response.headers[connection->currentField] += String(at, length);

	return 0;
}

int HttpConnection::staticOnBody(http_parser* parser, const char* at, size_t length)
{
	HttpConnection* connection = (HttpConnection*)parser->data;
	if(connection == nullptr) {
		// something went wrong
		return -1;
	}

	if(connection->incomingRequest->requestBodyDelegate) {
		return connection->incomingRequest->requestBodyDelegate(*connection, at, length);
	}

	if(connection->response.stream != nullptr) {
		int res = connection->response.stream->write((const uint8_t*)at, length);
		if(res != length) {
			// unable to write the requested bytes - stop here...
			delete connection->response.stream;
			connection->response.stream = nullptr;
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

REENTER:
	switch(state) {
	case eHCS_Ready: {
		HttpRequest* request = waitingQueue->peek();
		if(request == nullptr) {
			debug_d("Nothing in the waiting queue");
			outgoingRequest = nullptr;
			break;
		}

		// if the executionQueue is not empty then we have to check if we can pipeline that request
		if(executionQueue.count()) {
			if(!(request->method == HTTP_GET || request->method == HTTP_HEAD)) {
				// if the current request cannot be pipelined -> break;
				break;
			}

			// if we have previous request
			if(outgoingRequest != nullptr) {
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
		sendRequestHeaders(request);

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
		if(sendRequestBody(outgoingRequest)) {
			state = eHCS_Ready;
			delete stream;
			stream = nullptr;
			goto REENTER;
		}
	}
	} // switch(state)

	TcpClient::onReadyToSendData(sourceEvent);
}

void HttpConnection::sendRequestHeaders(HttpRequest* request)
{
	sendString(String(http_method_str(request->method)) + ' ' + request->uri.getPathWithQuery() + _F(" HTTP/1.1\r\n"));

	if(!request->headers.contains(HTTP_HEADER_HOST)) {
		request->headers[HTTP_HEADER_HOST] = request->uri.Host;
	}

	request->headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
	if(request->files.count()) {
		MultipartStream* mStream =
			new MultipartStream(HttpPartProducerDelegate(&HttpConnection::multipartProducer, this));
		request->headers[HTTP_HEADER_CONTENT_TYPE] =
			ContentType::toString(MIME_FORM_MULTIPART) + _F("; boundary=") + mStream->getBoundary();
		if(request->stream) {
			debug_e("HttpConnection: existing stream is discarded due to POST params");
			delete request->stream;
		}
		request->stream = mStream;
	} else if(request->postParams.count()) {
		UrlencodedOutputStream* uStream = new UrlencodedOutputStream(request->postParams);
		request->headers[HTTP_HEADER_CONTENT_TYPE] = ContentType::toString(MIME_FORM_URL_ENCODED);
		if(request->stream) {
			debug_e("HttpConnection: existing stream is discarded due to POST params");
			delete request->stream;
		}
		request->stream = uStream;
	} /* if (request->postParams.count()) */

	if(request->stream != nullptr) {
		if(request->stream->available() > -1) {
			request->headers[HTTP_HEADER_CONTENT_LENGTH] = String(request->stream->available());
		} else {
			request->headers.remove(HTTP_HEADER_CONTENT_LENGTH);
		}
	}

	if(!request->headers.contains(HTTP_HEADER_CONTENT_LENGTH)) {
		request->headers[HTTP_HEADER_TRANSFER_ENCODING] = _F("chunked");
	}

	for(unsigned i = 0; i < request->headers.count(); i++) {
		// TODO: add name and/or value escaping (implement in HttpHeaders)
		sendString(request->headers[i]);
	}
	sendString("\r\n");
}

bool HttpConnection::sendRequestBody(HttpRequest* request)
{
	if(state == eHCS_StartBody) {
		state = eHCS_SendingBody;

		if(request->stream == nullptr) {
			return true;
		}

		delete stream;
		if(request->headers[HTTP_HEADER_TRANSFER_ENCODING] == _F("chunked")) {
			stream = new ChunkedStream(request->stream);
		} else {
			stream = request->stream; // avoid intermediate buffers
		}
		request->stream = nullptr;
		return false;
	}

	if(stream == nullptr) {
		// we are done for now
		return true;
	}

	if(request->stream == nullptr && !stream->isFinished()) {
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
	return &response;
}

// end of public methods for HttpConnection

err_t HttpConnection::onReceive(pbuf* buf)
{
	if(buf == nullptr) {
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	pbuf* cur = buf;
	int parsedBytes = 0;
	while(cur != nullptr && cur->len > 0) {
		parsedBytes += http_parser_execute(&parser, &parserSettings, (char*)cur->payload, cur->len);
		if(HTTP_PARSER_ERRNO(&parser) != HPE_OK) {
			// we ran into trouble - abort the connection
			debug_e("HTTP parser error: %s", httpGetErrorName(HTTP_PARSER_ERRNO(&parser)).c_str());
			cleanup();
			TcpConnection::onReceive(nullptr);
			return ERR_ABRT;
		}

		cur = cur->next;
	}

	if(parser.upgrade) {
		return onProtocolUpgrade(&parser);
	} else if(parsedBytes != buf->tot_len) {
		TcpClient::onReceive(nullptr);

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
	reset();

	// if there are requests in the executionQueue -> move them back to the waiting queue
	for(unsigned i = 0; i < executionQueue.count(); i++) {
		waitingQueue->enqueue(executionQueue.dequeue());
	}
}

HttpConnection::~HttpConnection()
{
	cleanup();
}
