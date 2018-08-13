/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// Code based on http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform

#include "URL.h"
#include "../Services/WebHelpers/escape.h"

URL::URL(String urlString)
{
	unsigned len = urlString.length();

	// get query start
	int queryStart = urlString.indexOf('?');
	if (queryStart < 0)
		queryStart = len;

	// protocol
	int protocolEnd = urlString.indexOf(':'); // "://"
	if (protocolEnd >= 0 &&
		(((unsigned)protocolEnd + 3) < len && memcmp(urlString.c_str() + protocolEnd, _F("://"), 3) == 0)) {
		_protocol = urlString.substring(0, protocolEnd);
		_protocol.toLowerCase();
		protocolEnd += 3; //      Skip ://
	}
	else {
		// no protocol
		protocolEnd = 0;
		_protocol = DEFAULT_URL_PROTOCOL;
	}

	// host
	int hostStart = protocolEnd;
	int atStart = urlString.indexOf('@', protocolEnd);
	if (atStart > -1) {
		String credentials = urlString.substring(protocolEnd, atStart);
		Vector<String> parts;
		if (credentials.split(':', parts) == 2) {
			_user = parts[0];
			_password = parts[1];
		}
		else
			debug_e("URL: malformed credentials");
		hostStart = atStart + 1;
	}
	else {
		_user = nullptr;
		_password = nullptr;
	}

	// Port
	int pathStart = urlString.indexOf('/', hostStart);
	int portStart = urlString.indexOf(':', hostStart);
	int portEnd = (pathStart >= 0) ? pathStart : queryStart;
	bool hasPort = portStart >= 0 && portStart < portEnd;
	int hostEnd = portEnd;
	if (hasPort)
		hostEnd = portStart;

	_host = urlString.substring(hostStart, hostEnd);
	//debug_d("%d %d %s", hostStart, hostEnd, Host.c_str());

	// port
	if (hasPort) // we have a port
	{
		portStart++;
		_port = urlString.substring(portStart, portEnd).toInt();
	}
	else if (_protocol == HTTPS_URL_PROTOCOL || _protocol == WEBSCOKET_SECURE_URL_PROTOCOL)
		_port = 443;
	else if (_protocol == DEFAULT_URL_PROTOCOL)
		_port = 80;

	// path
	if (pathStart >= 0)
		_path = urlString.substring(pathStart, queryStart);
	else
		_path = nullptr;

	// query
	if (queryStart < (int)len)
		_query = urlString.substring(queryStart);
	else
		_query = nullptr;
}

String URL::toString() const
{
	String s = _protocol;
	s += _F("://");
	s += _host;
	if (_port)
		s += ':' + String(_port);
	s += pathWithQuery();
	return s;
}

String URL::pathWithQuery() const
{
	String s = _path + _query;
	if (s.length() == 0)
		s += '/';
	return s;
}

String URL::fileName() const
{
	int p = _path.lastIndexOf('/');
	return uri_unescape((p >= 0) ? _path.substring(p + 1) : _path);
}

HttpParams& URL::QueryParameters()
{
	if (!_queryParams) {
		_queryParams = new HttpParams();

		auto p = _query.c_str();
		if (p) {
			// Skip '?'
			++p;
			const char* pStart = p;
			const char* pSep = nullptr;

			auto add = [&]() {
				if (pSep) {
					String name(pStart, pSep - pStart);
					++pSep;
					String value(pSep, p - pSep);
					(*_queryParams)[name] = value;
				}
				else
					debug_w("getQueryParameter: Missing = in query string: %s", String(pStart, p - pStart).c_str());
				pStart = p + 1;
				pSep = nullptr;
			};

			while (*p) {
				if (*p == '&')
					add();
				else if (*p == '=')
					pSep = p;
				++p;
			}
			if (p > pStart)
				add();
		}
	}

	return *_queryParams;
}
