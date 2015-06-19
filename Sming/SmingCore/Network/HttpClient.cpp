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
	reset();
}

HttpClient::~HttpClient()
{
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

	if (uri.Protocol != "http") return false;
	debugf("Download: %s", uri.toString().c_str());

	connect(uri.Host, uri.Port);
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

void HttpClient::parseHeaders(pbuf* buf, int headerEnd)
{
	int line, nextLine;
	line = NetUtils::pbufFindStr(buf, "\r\n", 0) + 2;
	do
	{
		nextLine = NetUtils::pbufFindStr(buf, "\r\n", line);
		if (nextLine - line > 2)
		{
			int delim = NetUtils::pbufFindStr(buf, ":", line);
			if (delim != -1)
			{
				String name = NetUtils::pbufStrCopy(buf, line, delim - line);
				//if (server->isHeaderProcessingEnabled(name))
				{
					String value = NetUtils::pbufStrCopy(buf, delim + 1,
							nextLine - (delim + 1));
					value.trim();
					responseHeaders[name] = value;
					debugf("%s === %s", name.c_str(), value.c_str());
				}
			}
		}
		line = nextLine + 2;
	} while (nextLine != -1 && nextLine < headerEnd);
}

void HttpClient::writeRawData(pbuf* buf, int startPos)
{
	switch (mode)
	{
		case eHCM_String:
		{
			responseStringData += NetUtils::pbufStrCopy(buf, startPos,
					buf->tot_len - startPos);
			break;
		}
		case eHCM_File:
		{
			pbuf *cur = buf;
			while (cur != NULL && cur->len > 0 && !writeError)
			{
				char* ptr = (char*) cur->payload + startPos;
				int len = cur->len - startPos;
				int res = fileWrite(saveFile, ptr, len);
				writeError |= (res < 0);
				cur = cur->next;
				startPos = 0;
			}

			if (writeError)
				close();
		}
	}
}

err_t HttpClient::onReceive(pbuf *buf)
{
	if (buf == NULL)
	{
		// Disconnected, close it
		TcpClient::onReceive(buf);
	}
	else
	{
		int startPos = 0;
		if (waitParse)
		{
			String http = NetUtils::pbufStrCopy(buf, 0, 4);
			http.toUpperCase();
			if (http == "HTTP" && code == 0)
			{
				int codeStart = NetUtils::pbufFindChar(buf, ' ', 4);
				int codeEnd = NetUtils::pbufFindChar(buf, ' ', codeStart + 1);
				if (codeStart != -1 && codeEnd != -1 && codeEnd - codeStart < 5)
				{
					String strCode = NetUtils::pbufStrCopy(buf, codeStart, codeEnd);
					code = strCode.toInt();
				}
				else
					code = 0;
			}
			int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
			if (headerEnd != -1)
			{
				debugf("Header pos: %d", headerEnd);
				startPos = headerEnd + 4;
				waitParse = false;
				if (headerEnd < NETWORK_MAX_HTTP_PARSING_LEN)
					parseHeaders(buf, headerEnd);
			}
		}

		writeRawData(buf, startPos);

		// Fire ReadyToSend callback
		TcpClient::onReceive(buf);
	}

	return ERR_OK;
}

String HttpClient::getResponseString()
{
	if (mode == eHCM_String)
		return responseStringData;
	else
		return "";
}
