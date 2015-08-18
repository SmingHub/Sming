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
	postDataProcessed = 0;
	combinePostFrag = false;
}

HttpRequest::~HttpRequest()
{
	delete requestHeaders;
	delete requestGetParameters;
	delete requestPostParameters;
	delete cookies;
	postDataProcessed = 0;
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
	if (headerEnd == -1) return eHPR_Wait;
	if (headerEnd > NETWORK_MAX_HTTP_PARSING_LEN)
	{
		debugf("NETWORK_MAX_HTTP_PARSING_LEN");
		return eHPR_Failed;
	}

	int urlStart = NetUtils::pbufFindChar(buf, ' ') + 1;
	int urlEnd = NetUtils::pbufFindChar(buf, ' ', urlStart);
	if (urlStart == 0 || urlEnd == -1)
	{
		debugf("!BadRequest");
		return eHPR_Failed;
	}
	method = NetUtils::pbufStrCopy(buf, 0, urlStart - 1);

	int urlParamsStart = NetUtils::pbufFindChar(buf, '?', urlStart);
	if (urlParamsStart != -1 && urlParamsStart < urlEnd)
	{
		path = NetUtils::pbufStrCopy(buf, urlStart, urlParamsStart - urlStart);
		if (requestGetParameters == NULL) requestGetParameters = new HashMap<String, String>();
		extractParsingItemsList(buf, urlParamsStart + 1, urlEnd, '&', ' ', requestGetParameters);
	}
	else
		path = NetUtils::pbufStrCopy(buf, urlStart, urlEnd - urlStart);
	debugf("path=%s", path.c_str());

	int line, nextLine;
	line = NetUtils::pbufFindStr(buf, "\r\n", urlEnd)  + 2;
	do
	{
		nextLine = NetUtils::pbufFindStr(buf, "\r\n", line);
		if (nextLine - line > 2)
		{
			int delim = NetUtils::pbufFindStr(buf, ":", line);
			if (delim != -1)
			{
				String name = NetUtils::pbufStrCopy(buf, line, delim - line);
				if (server->isHeaderProcessingEnabled(name))
				{
					if (name == "Cookie")
					{
						if (cookies == NULL) cookies = new HashMap<String, String>();
						extractParsingItemsList(buf, delim + 1, nextLine, ';', '\r', cookies);
					}
					else
					{
						String value = NetUtils::pbufStrCopy(buf, delim + 1, nextLine - (delim + 1));
						value.trim();
						if (requestHeaders == NULL) requestHeaders = new HashMap<String, String>();
						(*requestHeaders)[name] = value;
						debugf("%s === %s", name.c_str(), value.c_str());
					}
				}
			}
		}
		line = nextLine + 2;
	} while(nextLine != -1);

	if (headerEnd != -1)
	{
		debugf("parsed");
		return eHPR_Successful;
	}
	return eHPR_Wait;
}

HttpParseResult HttpRequest::parsePostData(HttpServer *server, pbuf* buf)
{
	int start = 0;

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
		combinePostFrag = false;
	}
	else if (combinePostFrag)
	{
		String cur = requestPostParameters->keyAt(requestPostParameters->count() - 1);
		debugf("Continue POST frag %s", cur.c_str());
		int delimItem = NetUtils::pbufFindChar(buf, '&', 0);
		if (delimItem == -1)
			delimItem = buf->tot_len;
		else
			combinePostFrag = false;
		String itemValue = NetUtils::pbufStrCopy(buf, 0, delimItem);
		//debugf("Continue POST len %d", itemValue.length());
		char* buf = uri_unescape(NULL, 0, itemValue.c_str(), -1);
		itemValue = buf;
		free(buf);
		(*requestPostParameters)[cur] += itemValue;
		start = delimItem + 1;
		postDataProcessed += start;
	}

	bool notFinished = extractParsingItemsList(buf, start, buf->tot_len, '&', ' ', requestPostParameters);
	if (notFinished)
		combinePostFrag = true; // continue reading this parameter value
	//TODO: continue for param name
	postDataProcessed += buf->tot_len - start;

	if (postDataProcessed == getContentLength())
		return eHPR_Successful;
	else
		return eHPR_Wait;
}

bool HttpRequest::extractParsingItemsList(pbuf* buf, int startPos, int endPos, char delimChar, char endChar,
													HashMap<String, String>* resultItems)
{
	bool continued = false;
	int delimItem, nextItem, startItem = startPos;
	while (startItem < endPos)
	{
		delimItem = NetUtils::pbufFindStr(buf, "=", startItem);
		if (delimItem == -1 || delimItem > endPos) break;
		nextItem = NetUtils::pbufFindChar(buf, delimChar, delimItem + 1);
		if (nextItem == -1)
			nextItem = NetUtils::pbufFindChar(buf, endChar, delimItem + 1);
		if (nextItem > endPos) break;

		if (nextItem == -1)
		{
			nextItem = endPos;
			continued = true;
		}

		String ItemName = NetUtils::pbufStrCopy(buf, startItem, delimItem - startItem);
		String ItemValue = NetUtils::pbufStrCopy(buf, delimItem + 1, nextItem - delimItem - 1);
		char* nam = uri_unescape(NULL, 0, ItemName.c_str(), -1);
		ItemName = nam;
		free(nam);
		char* val = uri_unescape(NULL, 0, ItemValue.c_str(), -1);
		ItemValue = val;
		free(val);
		ItemName.trim();
		if (!continued) ItemValue.trim();
		debugf("Item: %s = %s", ItemName.c_str(), ItemValue.c_str());
		(*resultItems)[ItemName] = ItemValue;
		startItem = nextItem + 1;
	}
	return continued;
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
