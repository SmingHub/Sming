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
#include "Data/Stream/UrlencodedOutputStream.h"

#ifdef __linux__
#include "lwip/priv/tcp_priv.h"
#else
#include "lwip/tcp_impl.h"
#endif

static const http_parser_settings _parserSettings PROGMEM = {
	HTTP_PARSER_CALLBACK(HttpConnection, on_message_begin),
	HTTP_PARSER_CBNULL(HttpConnection, on_url),
#ifdef COMPACT_MODE
	HTTP_PARSER_CBNULL(HttpConnection, on_status),
#else
	HTTP_PARSER_CALLBACK(HttpConnection, on_status),
#endif
	HTTP_PARSER_CALLBACK(HttpConnection, on_header_field),
	HTTP_PARSER_CALLBACK(HttpConnection, on_header_value),
	HTTP_PARSER_CALLBACK(HttpConnection, on_headers_complete),
	HTTP_PARSER_CALLBACK(HttpConnection, on_body),
	HTTP_PARSER_CALLBACK(HttpConnection, on_message_complete),
#ifdef COMPACT_MODE
	HTTP_PARSER_CBNULL(HttpConnection, on_chunk_header),
	HTTP_PARSER_CBNULL(HttpConnection, on_chunk_complete),
#else
	HTTP_PARSER_CALLBACK(HttpConnection, on_chunk_header),
	HTTP_PARSER_CALLBACK(HttpConnection, on_chunk_complete)
#endif
};

HttpConnection::HttpConnection(RequestQueue* queue) : TcpClient(false)
{
	_waitingQueue = queue;

	http_parser_init(&_parser, HTTP_RESPONSE);
	_parser.data = this;
}

bool HttpConnection::connect(const String& host, int port, bool useSsl, uint32_t sslOptions)
{
	debug_d("HttpConnection::connect: TCP state: %d, isStarted: %d, isActive: %d", (_tcp ? _tcp->state : -1),
			getConnectionState() != eTCS_Ready, isActive());

	if (isProcessing())
		return true;

	if (getConnectionState() != eTCS_Ready && isActive()) {
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
	return _waitingQueue->enqueue(request);
}

bool HttpConnection::isActive()
{
	if (_tcp)
		for (tcp_pcb* pcb = tcp_active_pcbs; pcb; pcb = pcb->next)
			if (_tcp == pcb)
				return true;

	return false;
}

DateTime HttpConnection::getLastModifiedDate()
{
	DateTime res;
	return res.parseHttpDate(_response.headers[hhfn_LastModified]) ? res : DateTime();
}

DateTime HttpConnection::getServerDate()
{
	DateTime res;
	return res.parseHttpDate(_response.headers[hhfn_Date]) ? res : DateTime();
}

// @enddeprecated

void HttpConnection::reset()
{
	if (_incomingRequest) {
		delete _incomingRequest;
		_incomingRequest = nullptr;
	}

	_response.reset();

	_lastWasValue = true;
	_lastData = nullptr;
	_currentField = hhfn_UNKNOWN;
}

err_t HttpConnection::onProtocolUpgrade()
{
	debug_w("onProtocolUpgrade: Protocol upgrade is not supported");
	return ERR_ABRT;
}

int HttpConnection::on_message_begin()
{
	reset();

	_incomingRequest = _executionQueue.dequeue();

	// Return 1 if there are no requests in the queue
	return _incomingRequest ? 0 : 1;
}

int HttpConnection::on_message_complete()
{
	/* Return -2 if there is no current request
	 * @todo Why -2 ? The specific value is never acted upon or propagated by the parser.
	 */
	if (!_incomingRequest)
		return -2;

	debug_d("staticOnMessageComplete: Execution queue: %d, %s", _executionQueue.count(),
			_incomingRequest->uri.toString().c_str());

	/* we are finished with this request
	 * range of non-error codes are
	 * 	200 = HTTP_STATUS_OK
	 * 	399 = HTTP_STATUS_BAD_REQUEST - 1
	 */
	bool success = HTTP_PARSER_ERRNO(&_parser) == HPE_OK && _response.code >= 200 && _response.code <= 399;
	int err = _incomingRequest->RequestCompleted(*this, success);

	if (_incomingRequest->retries > 0) {
		_incomingRequest->retries--;
		return _executionQueue.enqueue(_incomingRequest) ? 0 : -1;
	}

	delete _incomingRequest;
	_incomingRequest = nullptr;

	if (_executionQueue.count() == 0)
		onConnected(ERR_OK);

	return err;
}

int HttpConnection::on_headers_complete()
{
	debug_d("The headers are complete");

	// nothing to process right now...
	if (!_incomingRequest)
		return HPE_HEADERS_NO_BODY;

	_response.code = _parser.status_code;

	int err = _incomingRequest->RequestHeadersCompleted(*this, _response);
	if (err)
		return err;

	// Don't send body for HEAD requests
	if (_incomingRequest->method == HTTP_HEAD)
		return HPE_HEADERS_NO_BODY;

	// Fetch body stream to send
	auto stream = _incomingRequest->getResponseStream();
	if (stream)
		_response.setResponseStream(stream);

	return 0;
}

#ifndef COMPACT_MODE

int HttpConnection::on_status(const char* at, size_t length)
{
	return 0;
}

#endif

int HttpConnection::on_header_field(const char* at, size_t length)
{
	if (_lastWasValue) {
		// we are starting to process new header
		_lastData = nullptr;
		_lastWasValue = false;
	}
	_lastData += String(at, length);

	return 0;
}

int HttpConnection::on_header_value(const char* at, size_t length)
{
	if (!_lastWasValue) {
		_currentField = HttpHeaders::fromString(_lastData);
		if (_currentField)
			_response.headers[_currentField] = "";
		_lastWasValue = true;
	}
	if (_currentField)
		_response.headers[_currentField] += String(at, length);

	return 0;
}

int HttpConnection::on_body(const char* at, size_t length)
{
	_incomingRequest->RequestBody(*this, at, length);
	return _response.bodyReceived(at, length) ? 0 : 1;
}

#ifndef COMPACT_MODE

int HttpConnection::on_chunk_header()
{
	debug_d("On chunk header");
	return 0;
}

int HttpConnection::on_chunk_complete()
{
	debug_d("On chunk complete");
	return 0;
}

#endif

void HttpConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	debug_d("HttpConnection::onReadyToSendData: waitingQueue.count: %d", _waitingQueue->count());

REENTER:
	switch (_state) {
	case eHCS_Ready: {
		HttpRequest* request = _waitingQueue->peek();
		if (!request) {
			debug_d("Nothing in the waiting queue");
			_outgoingRequest = nullptr;
			break;
		}

		// if the executionQueue is not empty then we have to check if we can pipeline that request
		if (_executionQueue.count()) {
			// Check to see if the current request can be pipelined
			if (request->method != HTTP_GET && request->method != HTTP_HEAD)
				break;

			// Does the outgoing request allow pipelining?
			if (_outgoingRequest && _outgoingRequest->method != HTTP_GET && _outgoingRequest->method != HTTP_HEAD)
				break;
		} // executionQueue.count()

		if (!_executionQueue.enqueue(request)) {
			debug_e("The working queue is full at the moment");
			break;
		}

		_state = eHCS_StartSending;
		// fall through
	}

	case eHCS_StartSending: {
		HttpRequest* request = _waitingQueue->dequeue();

		_outgoingRequest = request;

		// Send the request headers
		sendString(request->getRequestLine());

		auto& headers = request->prepareHeaders();
		for (unsigned i = 0; i < headers.count(); i++)
			sendString(headers[i]);
		sendString("\r\n");

		_state = eHCS_SendingHeaders;
		// fall through
	}

	case eHCS_SendingHeaders:
		if (isSending())
			break;
		_state = eHCS_StartBody;
		// fall through

	case eHCS_StartBody: {
		auto stream = _outgoingRequest->getBodyStream();
		if (!stream) {
			// No body, immediately start next request in queue
			_state = eHCS_Ready;
			goto REENTER;
		}
		TcpClient::send(stream);
		_state = eHCS_SendingBody;
		// fall through
	}

	case eHCS_SendingBody:
		//!! Why do we finish if request has a body to send ? Abort transfer, perhaps?
		if (_outgoingRequest->hasBody())
			debug_e("HttpConnection: OUTGOING REQUEST HAS BODY");
		else if (isSending())
			break;

		_state = eHCS_Sent;
		// fall through

	case eHCS_Sent:
		// immediately process next queued request (if any)
		_state = eHCS_Ready;
		goto REENTER;

	} // switch(state)

	TcpClient::onReadyToSendData(sourceEvent);
}

// end of public methods for HttpConnection

err_t HttpConnection::onReceive(pbuf* buf)
{
	// Disconnected, close it
	if (!buf)
		return TcpClient::onReceive(buf);

	auto cur = buf;
	size_t parsedBytes = 0;
	while (cur && cur->len > 0) {
		parsedBytes += http_parser_execute(&_parser, &_parserSettings, (char*)cur->payload, cur->len);
		auto errno = HTTP_PARSER_ERRNO(&_parser);
		if (errno != HPE_OK) {
			// we ran into trouble - abort the connection
			debug_e("HTTP parser error: %s", httpGetErrnoName(errno).c_str());
			cleanup();
			TcpConnection::onReceive(nullptr);
			return ERR_ABRT;
		}

		cur = cur->next;
	}

	if (_parser.upgrade)
		return onProtocolUpgrade();

	if (parsedBytes != buf->tot_len) {
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
	for (unsigned i = 0; i < _executionQueue.count(); i++)
		_waitingQueue->enqueue(_executionQueue.dequeue());
}

HttpConnection::~HttpConnection()
{
	cleanup();
}
