/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClientConnection.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpClientConnection.h"
#include "Data/Stream/FileStream.h"
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/LimitedMemoryStream.h"
#include "Data/Stream/ChunkedStream.h"
#include "Data/Stream/UrlencodedOutputStream.h"

bool HttpClientConnection::connect(const String& host, int port, bool useSsl)
{
	debug_d("HCC::connect: TCP state: %d, isStarted: %d, isActive: %d", (tcp != nullptr ? tcp->state : -1),
			(int)(getConnectionState() != eTCS_Ready), (int)isActive());

	if(isProcessing()) {
		return true;
	}

	if(getConnectionState() != eTCS_Ready && isActive()) {
		debug_d("HCC::connect: reusing TCP connection ");

		// we might have still alive connection
		onConnected(ERR_OK);
		return true;
	}

	debug_d("HCC::connect: connecting ...");

	return TcpClient::connect(host, port, useSsl);
}

bool HttpClientConnection::send(HttpRequest* request)
{
	if(!waitingQueue.enqueue(request)) {
		// the queue is full and we cannot add more requests at the time.
		debug_e("HCC::send: The request queue is full at the moment");
		delete request;
		return false;
	}

	bool useSsl = (request->uri.Scheme == URI_SCHEME_HTTP_SECURE);
	return connect(request->uri.Host, request->uri.getPort(), useSsl);
}

void HttpClientConnection::reset()
{
	incomingRequest = nullptr;

	response.reset();

	HttpConnection::reset();
}

int HttpClientConnection::onMessageBegin(http_parser* parser)
{
	incomingRequest = executionQueue.peek();
	if(incomingRequest == nullptr) {
		return 1; // there are no requests in the queue
	}

	return 0;
}

MultipartStream::BodyPart HttpClientConnection::multipartProducer()
{
	MultipartStream::BodyPart result;

	if(outgoingRequest->files.count()) {
		String name = outgoingRequest->files.keyAt(0);
		auto file = outgoingRequest->files.extractAt(0);
		result.stream = file;

		auto headers = new HttpHeaders();
		(*headers)[HTTP_HEADER_CONTENT_DISPOSITION] =
			F("form-data; name=\"") + name + F("\"; filename=\"") + file->getName() + '"';
		(*headers)[HTTP_HEADER_CONTENT_TYPE] = ContentType::fromFullFileName(file->getName());
		result.headers = headers;

		return result;
	}

	if(outgoingRequest->postParams.count()) {
		const String& name = outgoingRequest->postParams.keyAt(0);
		String& value = outgoingRequest->postParams.valueAt(0);

		result.stream = new MemoryDataStream(std::move(value));

		auto headers = new HttpHeaders();
		(*headers)[HTTP_HEADER_CONTENT_DISPOSITION] = F("form-data; name=\"") + name + '"';
		result.headers = headers;

		outgoingRequest->postParams.removeAt(0);
		return result;
	}

	return result;
}

int HttpClientConnection::onMessageComplete(http_parser* parser)
{
	if(!incomingRequest) {
		return -2; // no current request...
	}

	debug_d("HCC::onMessageComplete: executionQueue: %d, %s", executionQueue.count(),
			incomingRequest->uri.toString().c_str());

	// we are finished with this request
	int hasError = 0;
	if(incomingRequest->requestCompletedDelegate) {
		bool success = (HTTP_PARSER_ERRNO(parser) == HPE_OK) && // false when the parsing has failed
					   (response.isSuccess());					// false when the HTTP status code is not ok
		hasError = incomingRequest->requestCompletedDelegate(*this, success);
	}

	if(incomingRequest->retries > 0) {
		incomingRequest->retries--;
		return 0;
	}

	executionQueue.dequeue();

	delete incomingRequest;
	incomingRequest = nullptr;

	state = eHCS_Ready;

	auto response = getResponse();

	const String& headerConnection = static_cast<const HttpHeaders&>(response->headers)[HTTP_HEADER_CONNECTION];
	if(headerConnection.equalsIgnoreCase(_F("close"))) {
		allowPipe = false;
		// if the server does not support keep-alive -> close the connection
		// see: https://tools.ietf.org/html/rfc2616#section-14.10
		debug_d("HCC::onMessageComplete: Closing as requested by server");
		state = eHCS_WaitResponse; // put the other requests on hold...
		setTimeOut(0);			   // schedule for closing...

		return hasError;
	}

	allowPipe = true; // if the server supports keep-alive then it would most probably support also pipelining...

	if(executionQueue.count() == 0) {
		onConnected(ERR_OK);
	}

	return hasError;
}

int HttpClientConnection::onHeadersComplete(const HttpHeaders& headers)
{
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

	if(incomingRequest == nullptr) {
		// nothing to process right now...
		return 1;
	}

	response.headers.setMultiple(headers);
	response.code = HttpStatus(parser.status_code);

	if(incomingRequest->auth != nullptr) {
		incomingRequest->auth->setResponse(getResponse());
	}

	int error = 0;
	if(incomingRequest->headersCompletedDelegate) {
		error = incomingRequest->headersCompletedDelegate(*this, response);
	}

	if(!error && incomingRequest->method == HTTP_HEAD) {
		error = 1;
	}

	if(!error) {
		// set the response stream
		if(incomingRequest->responseStream != nullptr) {
			response.setBuffer(incomingRequest->responseStream);
			incomingRequest->responseStream = nullptr; // the response object will release that stream
		} else {
			response.setBuffer(new LimitedMemoryStream(NETWORK_SEND_BUFFER_SIZE));
		}
	}

	return error;
}

int HttpClientConnection::onBody(const char* at, size_t length)
{
	if(incomingRequest == nullptr) {
		// nothing to process right now...
		return 1;
	}

	if(incomingRequest->requestBodyDelegate) {
		return incomingRequest->requestBodyDelegate(*this, at, length);
	}

	if(response.buffer != nullptr) {
		auto res = response.buffer->write((const uint8_t*)at, length);
		if(res != length) {
			// unable to write the requested bytes - stop here...
			response.freeStreams();
			return 1;
		}
	}

	return 0;
}

void HttpClientConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	debug_d("HCC::onReadyToSendData: State: %d, executionQueue: %d, waitingQueue: %d", state, executionQueue.count(),
			waitingQueue.count());

REENTER:
	switch(state) {
	case eHCS_Ready: {
		HttpRequest* request = waitingQueue.peek();
		if(request == nullptr) {
			debug_d("HCC::onReadyToSendData: Nothing in the waiting queue");
			outgoingRequest = nullptr;
			break;
		}

		// if the executionQueue is not empty then we have to check if we can pipeline that request
		if(executionQueue.count() != 0) {
			if(!(allowPipe && (request->method == HTTP_GET || request->method == HTTP_HEAD))) {
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
			debug_e("HCC::onReadyToSendData: The working queue is full at the moment");
			break;
		}

		waitingQueue.dequeue();

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
			if(!(outgoingRequest->method == HTTP_GET || outgoingRequest->method == HTTP_HEAD)) {
				// we should wait for the response from this request.
				state = eHCS_WaitResponse;
				break;
			}

			state = eHCS_Ready;
			delete stream;
			stream = nullptr;
			goto REENTER;
		}
	}

	case eHCS_WaitResponse:
	default:; // Do nothing
	}		  // switch(state)

	TcpClient::onReadyToSendData(sourceEvent);
}

void HttpClientConnection::onClosed()
{
	if(waitingQueue.count() + executionQueue.count() > 0) {
		debug_d("HCC::onClosed: Trying to reconnect and send pending requests");

		cleanup();
		auto request = waitingQueue.peek();
		if(request != nullptr) {
			bool useSsl = (request->uri.Scheme == URI_SCHEME_HTTP_SECURE);
			connect(request->uri.Host, request->uri.getPort(), useSsl);
		}
	}
}

void HttpClientConnection::sendRequestHeaders(HttpRequest* request)
{
	String s = toString(request->method);
	s += ' ';
	s += request->uri.getPathWithQuery();
	s += _F(" HTTP/1.1\r\n");
	sendString(s);

	if(!request->headers.contains(HTTP_HEADER_HOST)) {
		request->headers[HTTP_HEADER_HOST] = request->uri.getHostWithPort();
	}

	request->headers[HTTP_HEADER_CONTENT_LENGTH] = "0";
	if(request->files.count()) {
		auto mStream = new MultipartStream(MultipartStream::Producer(&HttpClientConnection::multipartProducer, this));
		s = toString(MIME_FORM_MULTIPART);
		s += F("; boundary=");
		s += mStream->getBoundary();
		request->headers[HTTP_HEADER_CONTENT_TYPE] = s;
		if(request->bodyStream != nullptr) {
			debug_e("HCC::sendRequestHeaders: existing stream is discarded due to POST params");
			delete request->bodyStream;
		}
		request->bodyStream = mStream;
	} else if(request->postParams.count() != 0) {
		UrlencodedOutputStream* uStream = new UrlencodedOutputStream(request->postParams);
		request->headers[HTTP_HEADER_CONTENT_TYPE] = toString(MIME_FORM_URL_ENCODED);
		if(request->bodyStream) {
			debug_e("HCC::sendRequestHeaders: existing stream is discarded due to POST params");
			delete request->bodyStream;
		}
		request->bodyStream = uStream;
	} /* if (request->postParams.count()) */

	if(request->bodyStream != nullptr) {
		if(request->bodyStream->available() > -1) {
			request->headers[HTTP_HEADER_CONTENT_LENGTH] = String(request->bodyStream->available());
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

bool HttpClientConnection::sendRequestBody(HttpRequest* request)
{
	if(state == eHCS_StartBody) {
		state = eHCS_SendingBody;

		if(request->bodyStream == nullptr) {
			return true;
		}

		delete stream;
		if(request->headers[HTTP_HEADER_TRANSFER_ENCODING] == _F("chunked")) {
			stream = new ChunkedStream(request->bodyStream);
		} else {
			stream = request->bodyStream; // avoid intermediate buffers
		}
		request->bodyStream = nullptr;
		return false;
	}

	if(stream == nullptr) {
		// we are done for now
		return true;
	}

	if(request->bodyStream == nullptr && !stream->isFinished()) {
		return false;
	}

	return true;
}

// end of public methods for HttpClientConnection

void HttpClientConnection::cleanup()
{
	reset();

	// if there are requests in the executionQueue -> move them back to the waiting queue
	while(executionQueue.count() != 0) {
		waitingQueue.enqueue(executionQueue.dequeue());
	}
}
