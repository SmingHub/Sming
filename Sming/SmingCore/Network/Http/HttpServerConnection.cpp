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
#include "Data/Stream/ChunkedStream.h"

#define HTTP_SERVER_NAME F("HttpServer/Sming")

static const http_parser_settings _parserSettings PROGMEM = {
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_message_begin),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_url),
	HTTP_PARSER_CBNULL(HttpServerConnection, on_status),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_header_field),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_header_value),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_headers_complete),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_body),
	HTTP_PARSER_CALLBACK(HttpServerConnection, on_message_complete),
	HTTP_PARSER_CBNULL(HttpServerConnection, on_chunk_header),
	HTTP_PARSER_CBNULL(HttpServerConnection, on_chunk_complete)};

int HttpServerConnection::on_message_begin()
{
	_response.reset();

	_state = eHCS_Ready;

	// ... and Request
	_request.setMethod((HttpMethod)_parser.method);

	// and temp data...
	_request.headers.clear();
	_bodyParser = nullptr;

	return 0;
}

int HttpServerConnection::on_url(const char* at, size_t length)
{
	// TODO: find the most suitable path..

	String path = String(at, length);
	if (path.length() > 1 && path.endsWith("/"))
		path.remove(path.length() - 1);

	_request.setURL(path);

	_resource = _resourceTree.find(_request.uri.path());
	if (!_resource)
		_resource = _resourceTree.find("*");

	return 0;
}

int HttpServerConnection::on_message_complete()
{
	// we are finished with this request
	auto err = HTTP_PARSER_ERRNO(&_parser);
	if (err != HPE_OK) {
		sendError(httpGetErrnoName(err));
		return 0;
	}

	if (_bodyParser)
		_bodyParser(_request, nullptr, -2);

	int res = 0;
	if (_resource && _resource->onRequestComplete)
		res = _resource->onRequestComplete(*this, _request, _response);

	send();

	delete _request.getResponseStream();

	return res;
}

int HttpServerConnection::on_headers_complete()
{
	debug_d("The headers are complete");

	int error = 0;

	_lastWasValue = true;
	_lastData = nullptr;
	_currentField = hhfn_UNKNOWN;

	if (_resource && _resource->onHeadersComplete)
		error = _resource->onHeadersComplete(*this, _request, _response);

	if (!error && _request.method == HTTP_HEAD)
		error = HPE_HEADERS_NO_BODY;

	if (_request.headers.contains(hhfn_ContentType)) {
		String contentType = _request.headers[hhfn_ContentType];
		int endPos = contentType.indexOf(';');
		if (endPos >= 0)
			contentType = contentType.substring(0, endPos);

		String majorType = contentType.substring(0, contentType.indexOf('/'));
		majorType += _F("/*");

		// Content-Type for exact type: application/json
		// Wildcard type for application: application/*
		// Wildcard type for the rest*

		Vector<String> types;
		types.add(contentType);
		types.add(majorType);
		types.add("*");

		for (unsigned i = 0; i < types.count(); i++)
			if (_bodyParsers.contains(types.at(i))) {
				_bodyParser = _bodyParsers[types.at(i)];
				break;
			}

		if (_bodyParser)
			_bodyParser(_request, nullptr, -1);
	}

	return error;
}

int HttpServerConnection::on_header_field(const char* at, size_t length)
{
	if (_lastWasValue) {
		// we are starting to process new header
		_lastData = nullptr;
		_lastWasValue = false;
	}

	_lastData += String(at, length);
	return 0;
}

int HttpServerConnection::on_header_value(const char* at, size_t length)
{
	if (!_lastWasValue) {
		_currentField = HttpHeaders::fromString(_lastData);
		if (_currentField)
			_request.headers[_currentField] = "";
		_lastWasValue = true;
	}

	if (_currentField)
		_request.headers[_currentField] += String(at, length);

	return 0;
}

int HttpServerConnection::on_body(const char* at, size_t length)
{
	if (_bodyParser)
		_bodyParser(_request, at, length);

	if (_resource && _resource->onBody)
		return _resource->onBody(*this, _request, at, length);

	return 0;
}

err_t HttpServerConnection::onReceive(pbuf* buf)
{
	// close the connection on TCP error
	if (!buf)
		return TcpConnection::onReceive(nullptr);

	err_t err = ERR_OK;

	pbuf* cur = buf;

	int parsedBytes = 0;
	while (!_parser.upgrade && cur && cur->len > 0) {
		parsedBytes += http_parser_execute(&_parser, &_parserSettings, (const char*)cur->payload, cur->len);
		auto errno = HTTP_PARSER_ERRNO(&_parser);
		if (errno != HPE_OK) {
			// we ran into trouble - abort the connection
			debug_e("HTTP parser error: %s", httpGetErrnoName(errno).c_str());
			sendError();

			if (errno >= HPE_INVALID_EOF_STATE)
				err = ERR_ABRT;

			break;
		}

		cur = cur->next;
	}

	// Any remaining bytes must be for upgrade connection
	if (!err && parsedBytes < buf->tot_len) {
		if (!_parser.upgrade) {
			err = ERR_ABRT;
		}
		else if (_resource && _resource->onUpgrade) {
			while (cur && cur->len > 0) {
				int uperr = _resource->onUpgrade(*this, _request, (char*)cur->payload, cur->len);
				if (uperr) {
					debug_e("The upgraded connection returned error: %d", uperr);
					err = ERR_ABRT;
					break;
				}
				cur = cur->next;
			}
		}
	}

	TcpConnection::onReceive(err ? nullptr : buf);
	return err;
}

void HttpServerConnection::onReadyToSendData(TcpConnectionEvent sourceEvent)
{
	switch (_state) {
	case eHCS_StartSending: {
		auto& headers = prepareHeaders();
		for (unsigned i = 0; i < headers.count(); i++)
			sendString(headers[i]);
		sendString("\r\n");

		_state = eHCS_SendingHeaders;
		// Fall through
	}

	case eHCS_SendingHeaders:
		if (isSending())
			break;
		_state = eHCS_StartBody;
		// Fall through

	case eHCS_StartBody:
		// For HEAD request we don't send a body
		if (_request.method == HTTP_HEAD)
			_response.freeStreams();
		else {
			auto stream = _response.getBodyStream();
			if (stream)
				send(stream);
		}

		_state = eHCS_SendingBody;
		// fall through

	case eHCS_SendingBody:
		/*
		 * @todo explain. Why do we finish if response has a body to send ?
		 * This seems like an error condition so an assert() may be appropriate.
		 */
		if (_response.hasBody())
			debug_e("HttpServerConnection::sendRequestBody(): OUTGOING RESPONSE HAS BODY");
		else if (isSending())
			break;

		_state = eHCS_Sent;
		// Fall through

	case eHCS_Sent:
		// If requested, close connection asynchronously
		if (_response.headers[hhfn_Connection] == _F("close"))
			setTimeOut(1);

		_response.reset();
		_request.reset();

		_state = eHCS_Ready;
		// Fall through

	case eHCS_Ready:
		// Ready to process the next incoming request
		;

	} /* switch(state) */

	TcpClient::onReadyToSendData(sourceEvent);
}

/*
 * Prepare response headers for sending and return them.
 */
HttpHeaders& HttpServerConnection::prepareHeaders()
{
	auto& headers = _response.prepareHeaders();

#ifndef DISABLE_HTTPSRV_ETAG
	if (_request.headers.contains(hhfn_IfMatch) && headers.contains(hhfn_ETag) &&
		_request.headers[hhfn_IfMatch] == headers[hhfn_ETag]) {
		if (_request.method == HTTP_GET || _request.method == HTTP_HEAD) {
			_response.code = HTTP_STATUS_NOT_MODIFIED;
			headers[hhfn_ContentLength] = "0";
			delete _response.getBodyStream();
		}
	}
#endif /* DISABLE_HTTPSRV_ETAG */

	sendString(_response.getStatusLine());

	if (!headers.contains(hhfn_Connection)) {
		if (_request.headers[hhfn_Connection] == F("close")) {
			// the other side requests closing of the tcp connection...
			headers[hhfn_Connection] = F("close");
		}
		else {
			// Keep-Alive to reuse the connection
			headers[hhfn_Connection] = F("keep-alive");
		}
	}

#if HTTP_SERVER_EXPOSE_NAME == 1
	headers[hhfn_Server] = HTTP_SERVER_NAME;
#endif

#if HTTP_SERVER_EXPOSE_DATE == 1
	headers[hhfn_Date] = SystemClock.getSystemTimeString();
#endif

	return headers;
}

void HttpServerConnection::onError(err_t err)
{
	TcpClient::onError(err);
}

void HttpServerConnection::send()
{
	_state = eHCS_StartSending;
	onReadyToSendData(eTCE_Received);
}

void HttpServerConnection::sendError(const String& message, enum http_status code /* = HTTP_STATUS_BAD_REQUEST */)
{
	debug_d("SEND ERROR PAGE");
	_response.code = code;
	_response.setContentType(MIME_HTML);

	String html = F("<H2 color='#444'>");
	html += message ?: httpGetStatusText(_response.code);
	html += _F("</H2>");
	_response.headers[hhfn_ContentLength] = html.length();
	_response.sendString(html);

	send();
}
