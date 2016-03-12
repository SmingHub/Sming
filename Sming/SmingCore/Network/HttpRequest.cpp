/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#include "HttpRequest.h"
#include "HttpServer.h"
#include "NetUtils.h"
#include <stdlib.h>
#include "../../Services/WebHelpers/escape.h"

HttpRequest::HttpRequest()
{
	requestHeaders = NULL;
	requestGetParameters = NULL;
	requestPostParameters = NULL;
	cookies = NULL;
	headerDataProcessed = 0;
	postDataProcessed = 0;
	bodyBuf = NULL;
	tmpbuf = "";
}

HttpRequest::~HttpRequest()
{
	delete requestHeaders;
	delete requestGetParameters;
	delete requestPostParameters;
	delete cookies;
	postDataProcessed = 0;
	if (bodyBuf != NULL)
	{
		os_free(bodyBuf);
	}
}

String HttpRequest::getQueryParameter(String parameterName, String defaultValue /* = "" */)
{
	if (requestGetParameters && requestGetParameters->contains(parameterName))
			return (*requestGetParameters)[parameterName];

	return defaultValue;
}
String HttpRequest::getPostParameter(String parameterName, String defaultValue /* = "" */)
{
	if (requestPostParameters && requestPostParameters->contains(parameterName))
			return (*requestPostParameters)[parameterName];

	return defaultValue;
}

String HttpRequest::getHeader(String headerName, String defaultValue /* = "" */)
{
	if (requestHeaders && requestHeaders->contains(headerName))
		return (*requestHeaders)[headerName];

	return defaultValue;
}

String HttpRequest::getCookie(String name, String defaultValue /* = "" */)
{
	if (cookies && cookies->contains(name))
		return (*cookies)[name];

	return defaultValue;
}

int HttpRequest::getContentLength()
{
	String len = getHeader("Content-Length");
	if (len.length() == 0) return -1;

	return len.toInt();
}

String HttpRequest::getContentType()
{
	return getHeader("Content-Type");
}

HttpParseResult HttpRequest::parseHeader(HttpServer *server, pbuf* buf)
{
	int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
	if (headerEnd > NETWORK_MAX_HTTP_PARSING_LEN || headerDataProcessed > NETWORK_MAX_HTTP_PARSING_LEN \
		|| (headerEnd != -1 && buf->tot_len > NETWORK_MAX_HTTP_PARSING_LEN))
	{
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return eHPR_Failed;
	}
	int urlEnd = 0;
	tmpbuf += NetUtils::pbufStrCopy(buf, 0, buf->tot_len);
	if (requestHeaders == NULL) {
		// first time calling header
		requestHeaders = new HashMap<String, String>();

		int urlStart = tmpbuf.indexOf(" ")+1;
		urlEnd = tmpbuf.indexOf(" ", urlStart);
		if (urlStart == 0 || urlEnd == -1)
		{
			debugf("!BadRequest");
			return eHPR_Failed;
		}

		method = tmpbuf.substring(0, urlStart-1);
		int urlParamsStart = tmpbuf.indexOf("?", urlStart);
		if (urlParamsStart != -1 && urlParamsStart < urlEnd)
		{
			path = tmpbuf.substring(urlStart, urlParamsStart);
			if (requestGetParameters == NULL) requestGetParameters = new HashMap<String, String>();
			extractParsingItemsList(tmpbuf, urlParamsStart + 1, urlEnd, '&', ' ', requestGetParameters);
		}
		else
			path = tmpbuf.substring(urlStart, urlEnd);
		debugf("path=%s", path.c_str());
		urlEnd = tmpbuf.indexOf("\r\n", urlEnd)+2;
	}

	int line, nextLine;
	line = urlEnd;
	do
	{

		nextLine = tmpbuf.indexOf("\r\n", line);
		if (nextLine - line > 2)

		{
			int delim = tmpbuf.indexOf(":", line);
			if (delim != -1)
			{
				String name = tmpbuf.substring(line, delim);
				if (server->isHeaderProcessingEnabled(name))
				{
					debugf("Name: %s", name.c_str());
					if (name == "Cookie")
					{
						if (cookies == NULL) cookies = new HashMap<String, String>();
						extractParsingItemsList(tmpbuf, delim + 1, nextLine, ';', '\r', cookies);
					}
					else
					{
						String value = tmpbuf.substring(delim + 1, nextLine);
						value.trim();
						(*requestHeaders)[name] = value;
						debugf("%s === %s", name.c_str(), value.c_str());
					}

				}
			}

		}
		if (nextLine != -1) {
			line = nextLine + 2;
		}

	} while(nextLine != -1);

	if (headerEnd != -1)
	{
		tmpbuf = "";
		debugf("parsed");
		return eHPR_Successful;
	}
	headerDataProcessed += buf->tot_len;
	tmpbuf = tmpbuf.substring(line, buf->tot_len);
	return eHPR_Wait;
}

HttpParseResult HttpRequest::parsePostData(HttpServer *server, pbuf* buf)
{
	int start = 0;
	tmpbuf += NetUtils::pbufStrCopy(buf, 0, buf->tot_len);
	// First enter
	if (requestPostParameters == NULL)
	{
		int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
		if (headerEnd == -1) return eHPR_Failed;
		if (headerEnd + getContentLength() > NETWORK_MAX_HTTP_PARSING_LEN)
		{
			debugf("NETWORK_MAX_HTTP_PARSING_LEN");
			return eHPR_Failed;
		}
		requestPostParameters = new HashMap<String, String>();
		start = headerEnd + 4;
		tmpbuf = tmpbuf.substring(start, tmpbuf.length());
	}

	tmpbuf = extractParsingItemsList(tmpbuf, 0, tmpbuf.length(), '&', ' ', requestPostParameters);
	postDataProcessed += buf->tot_len - start ;

	if (postDataProcessed == getContentLength())
		return eHPR_Successful;
	else

		return eHPR_Wait;
}

String HttpRequest::extractParsingItemsList(String& buf, int startPos, int endPos, char delimChar, char endChar,
													HashMap<String, String>* resultItems)
{

	int delimItem, nextItem, startItem = startPos;
	do
	{
		delimItem = buf.indexOf("=", startItem);
		nextItem = buf.indexOf(delimChar, startItem);
		//debugf("item %i  - delim %i - next %i", startItem, delimItem, nextItem);
		if (nextItem == -1) nextItem = buf.indexOf(endChar, delimItem+1);
		if (nextItem == -1) nextItem = endPos;
		if (nextItem > endPos || delimItem == -1) nextItem = endPos;
		if (delimItem == -1) break;
		String ItemName = buf.substring(startItem, delimItem);
		String ItemValue = buf.substring(delimItem+1, nextItem);
		char* nam = uri_unescape(NULL, 0, ItemName.c_str(), -1);
		ItemName = nam;
		free(nam);
		char* val = uri_unescape(NULL, 0, ItemValue.c_str(), -1);
		ItemValue = val;
		free(val);
		ItemName.trim();
		debugf("Item: Name = %s, Size = %d, Value = %s",ItemName.c_str(),ItemValue.length(),ItemValue.substring(0,80).c_str());
		(*resultItems)[ItemName] = ItemValue;
		if (nextItem == endPos) break;
		startItem = nextItem + 1;


	} while (nextItem != -1);
	return tmpbuf.substring(startItem, nextItem);

}

void HttpRequest::parseRawData(HttpServer *server, pbuf* buf)
{
	bodyBuf = (char *) os_zalloc(sizeof(char) * buf->tot_len);
	int headerEnd = NetUtils::pbufFindStr(buf, "\r\n\r\n");
	if (headerEnd + getContentLength() > NETWORK_MAX_HTTP_PARSING_LEN)
	{
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return;
	}
	pbuf_copy_partial(buf, bodyBuf, buf->tot_len, headerEnd + 4);
}

char* HttpRequest::getBody()
{
	return bodyBuf;
}

bool HttpRequest::isAjax()
{
	String req = getHeader("HTTP_X_REQUESTED_WITH");
	return req.equalsIgnoreCase("xmlhttprequest");
}

bool HttpRequest::isWebSocket()
{
	String req = getHeader("Upgrade");
	return req.equalsIgnoreCase("websocket");
}
