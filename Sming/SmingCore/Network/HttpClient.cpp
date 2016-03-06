/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpClient.h"

#include "../SmingCore.h"

HttpClient::HttpClient(bool autoDestruct /* = false */) : TcpClient(autoDestruct)
{
	parser = (http_parser *)malloc(sizeof(http_parser));
	reset();
}

HttpClient::HttpClient(ResponseBodyDelegate responseBodyDelegate, bool autoDestruct /* = false */) : HttpClient(autoDestruct)
{
	this->responseBodyDelegate = responseBodyDelegate;
}

HttpClient::HttpClient(HeadersCompleteDelegate headersCompleteDelegate,
			   ResponseBodyDelegate responseBodyDelegate /* = NULL */, bool autoDestruct /* = false */) : HttpClient(autoDestruct)
{
	this->headersCompleteDelegate = headersCompleteDelegate;
	if(responseBodyDelegate) {
		this->responseBodyDelegate = responseBodyDelegate;
	}
}

HttpClient::~HttpClient()
{
	if(parser != NULL) {
		free(parser);
	}
}

bool HttpClient::downloadString(String url, HttpClientCompletedDelegate onCompleted)
{
	if (isProcessing()) return false;
	URL uri = URL(url);

	return startDownload(uri, eHCM_String, onCompleted);
}

bool HttpClient::downloadFile(String url, HttpClientCompletedDelegate onCompleted /* = NULL */)
{
	return downloadFile(url, "", onCompleted);
}

bool HttpClient::downloadFile(String url, String saveFileName, HttpClientCompletedDelegate onCompleted /* = NULL */)
{
	if (isProcessing()) return false;
	URL uri = URL(url);

	String file;
	if (saveFileName.length() == 0)
	{
		file = uri.Path;
		int p = file.lastIndexOf('/');
		if (p != -1)
			file = file.substring(p + 1);
	}
	else
		file = saveFileName;

	saveFile = fileOpen(file.c_str(), eFO_CreateNewAlways | eFO_WriteOnly);
	debugf("Download file: %s %d", file.c_str(), saveFile);

	return startDownload(uri, eHCM_File, onCompleted);
}

bool HttpClient::startDownload(URL uri, HttpClientMode mode, HttpClientCompletedDelegate onCompleted)
{
	reset();
	this->mode = mode;
	this->onCompleted = onCompleted;

	debugf("Download: %s", uri.toString().c_str());

	if(uri.Protocol == HTTPS_URL_PROTOCOL) {
		connect(uri.Host, uri.Port, true);
	}
	else {
		connect(uri.Host, uri.Port);
	}

	bool isPost = body.length();

	sendString((isPost ? "POST " : "GET ") + uri.getPathWithQuery() + " HTTP/1.0\r\nHost: " + uri.Host + "\r\n");
	for (int i = 0; i < requestHeaders.count(); i++)
	{
		String write = requestHeaders.keyAt(i) + ": " + requestHeaders.valueAt(i) + "\r\n";
		sendString(write.c_str());
	}
	sendString("\r\n");
	sendString(body);

	return true;
}

void HttpClient::setRequestHeader(const String name, const String value)
{
	requestHeaders[name] = value;
}

bool HttpClient::hasRequestHeader(const String name)
{
	return requestHeaders.contains(name);
}

void HttpClient::setRequestContentType(String contentType)
{
    setRequestHeader("Content-Type", contentType);
}

void HttpClient::setPostBody(const String& _body)
{
    if (!hasRequestHeader("Content-Type"))
    	setRequestContentType(ContentType::FormUrlEncoded);
    body = _body;
    setRequestHeader("Content-Length", String(body.length()));
}


String HttpClient::getPostBody()
{
    return body;
}

void HttpClient::reset()
{
	code = 0;
	responseStringData = "";
	waitParse = true;
	writeError = false;
	responseHeaders.clear();

	lastWasValue = true;
	lastData = "";
	currentField  = "";
	totalHeadersSize = 0;
}

HashMap<String, String> &HttpClient::getResponseHeaders()
{
	return responseHeaders;
}

String HttpClient::getResponseHeader(String headerName, String defaultValue /* = "" */)
{
	if (responseHeaders.contains(headerName))
		return responseHeaders[headerName];

	return defaultValue;
}

DateTime HttpClient::getLastModifiedDate()
{
	DateTime res;
	String strLM = getResponseHeader("Last-Modified");
	if (res.parseHttpDate(strLM))
		return res;
	else
		return DateTime();
}

DateTime HttpClient::getServerDate()
{
	DateTime res;
	String strSD = getResponseHeader("Date");
	if (res.parseHttpDate(strSD))
		return res;
	else
		return DateTime();
}

void HttpClient::onFinished(TcpClientState finishState)
{
	if (finishState == eTCS_Failed) code = 0;

	if (mode == eHCM_File)
	{
		debugf("Download file len written: %d, res^ %d", fileTell(saveFile), isSuccessful());
		if (!isSuccessful())
			fileDelete(saveFile);
		fileClose(saveFile);
	}

	if (onCompleted)
		onCompleted(*this, isSuccessful());

	TcpClient::onFinished(finishState);
}

err_t HttpClient::onResponseBody(const char *at, size_t length)
{
	if(responseBodyDelegate) {
		return responseBodyDelegate(*this, at, length);
	}

	switch (mode)
	{
		case eHCM_String:
		{
			responseStringData += String(at, length);
			break;
		}
		case eHCM_File:
		{
			int res = fileWrite(saveFile, &at, length);
			writeError |= (res < 0);

			if (writeError)
				close();
		}
	}

	return ERR_OK;
}

err_t HttpClient::onProtocolUpgrade(http_parser* parser)
{
	debugf("HttpClient::onProtocolUpgrade: Protocol upgrade is not supported");
	return ERR_ABRT;
}

int HttpClient::staticOnMessageComplete(http_parser* parser)
{
	debugf("HttpClient::staticOnMessageComplete: .");

	return 0;
}

int HttpClient::staticOnHeadersComplete(http_parser* parser)
{
	HttpClient *client = (HttpClient*)parser->data;
	if(client == NULL) {
		// something went wrong
		return -1;
	}

	client->code = parser->status_code;

	debugf("The headers are complete");
	if(client->headersCompleteDelegate) {
		return client->headersCompleteDelegate(*client);
	}

	return 0;
}

int GDB_IRAM_ATTR HttpClient::staticOnHeaderField(http_parser *parser, const char *at, size_t length)
{
	HttpClient *client = (HttpClient*)parser->data;
	if(client == NULL) {
		// something went wrong
		return -1;
	}

	if(client->lastWasValue) {
		// we are starting to process new header
		client->lastData = "";
		client->lastWasValue = false;
	}
	client->lastData += String(at, length);

	return 0;
}

int HttpClient::staticOnHeaderValue(http_parser *parser, const char *at, size_t length)
{
	HttpClient *client = (HttpClient*)parser->data;
	if (client == NULL) {
		// something went wrong
		return -1;
	}

	if(!client->lastWasValue) {
		client->currentField = client->lastData;
		client->responseHeaders[client->currentField] = "";
		client->lastWasValue = true;
	}
	client->responseHeaders[client->currentField] += String(at, length);

	return 0;
}

int HttpClient::staticOnBody(http_parser *parser, const char *at, size_t length)
{
	HttpClient *client = (HttpClient*)parser->data;
	if (client == NULL) {
		// something went wrong
		return -1;
	}

	client->onResponseBody(at, length);

	return 0;
}

err_t GDB_IRAM_ATTR HttpClient::onConnected(err_t err)
{
	if (err == ERR_OK)
	{
		http_parser_init(parser, HTTP_RESPONSE);
		parser->data = (void*)this;

		// Callbacks: on_message_begin, on_headers_complete, on_message_complete.
		parserSettings.on_message_begin     = staticOnMessageComplete;
		parserSettings.on_message_complete  = staticOnMessageComplete;
		parserSettings.on_headers_complete  = staticOnHeadersComplete;

		// on_url, (common) on_header_field, on_header_value, on_body;
		parserSettings.on_header_field      = staticOnHeaderField;
		parserSettings.on_header_value      = staticOnHeaderValue;
		parserSettings.on_body              = staticOnBody;
	}

	// Fire ReadyToSend callback
	TcpClient::onConnected(err);

	return ERR_OK;
}

err_t GDB_IRAM_ATTR HttpClient::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		return TcpClient::onReceive(buf);
	}

	/* Basic sanity check */
	totalHeadersSize += buf->tot_len;
	if(totalHeadersSize > MAX_HTTP_HEADERS_SIZE) {
		return ERR_ABRT;
	}

	char *data = (char *)malloc(buf->tot_len);
	pbuf_copy_partial(buf, data, buf->tot_len,0 );
	int parsedBytes = http_parser_execute(parser, &parserSettings, data, buf->tot_len);
	free(data);

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

String HttpClient::getResponseString()
{
	if (mode == eHCM_String)
		return responseStringData;
	else
		return "";
}
