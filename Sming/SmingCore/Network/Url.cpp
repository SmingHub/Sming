/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Url.cpp
 *
 * Code based on http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform
 *
 ****/

#include "Url.h"

Url::Url()
{
	Port = 80;
}

Url::Url(const String& urlString)
{
	int len = urlString.length();
	if(len == 0)
		return;

	// get query start
	int queryStart = urlString.indexOf('?');
	if(queryStart == -1)
		queryStart = len;

	// protocol
	int protocolEnd = urlString.indexOf(':'); //"://");

	if(protocolEnd != -1) {
		if((len - protocolEnd > 3) && (urlString.substring(protocolEnd, protocolEnd + 3) == "://")) {
			Protocol = urlString.substring(0, protocolEnd);
			Protocol.toLowerCase();
			protocolEnd += 3; //      Skip ://
		} else {
			protocolEnd = 0; // no protocol
			Protocol = DEFAULT_URL_PROTOCOL;
		}
	} else {
		protocolEnd = 0; // no protocol
		Protocol = DEFAULT_URL_PROTOCOL;
	}

	// host
	int hostStart = protocolEnd;
	int atStart = urlString.indexOf('@', protocolEnd);
	if(atStart > -1) {
		String credentials = urlString.substring(protocolEnd, atStart);
		Vector<String> parts;
		splitString(credentials, ':', parts);
		User = parts[0];
		Password = parts[1];
		hostStart = atStart + 1;
	}

	int pathStart = urlString.indexOf('/', hostStart); // get pathStart

	int portStart = urlString.indexOf(':', hostStart); // check for port
	int portEnd = (pathStart != -1) ? pathStart : queryStart;
	bool hasPort = portStart != -1 && portStart < portEnd;
	int hostEnd = portEnd;
	if(hasPort)
		hostEnd = portStart;

	Host = urlString.substring(hostStart, hostEnd);
	//debug_d("%d %d %s", hostStart, hostEnd, Host.c_str());

	// port
	if(hasPort) // we have a port
	{
		portStart++;
		Port = urlString.substring(portStart, portEnd).toInt();
	} else if(Protocol == HTTPS_URL_PROTOCOL || Protocol == WEBSOCKET_SECURE_URL_PROTOCOL) {
		Port = 443;
	} else if(Protocol == DEFAULT_URL_PROTOCOL) {
		Port = 80;
	}

	// path
	if(pathStart != -1)
		Path = urlString.substring(pathStart, queryStart);

	// query
	if(queryStart != len)
		Query = urlString.substring(queryStart);
}
