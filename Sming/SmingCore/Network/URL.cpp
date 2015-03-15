/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

// Code based on http://stackoverflow.com/questions/2616011/easy-way-to-parse-a-url-in-c-cross-platform

#include "URL.h"

URL::URL()
{
	Port = 80;
}

URL::URL(String urlString)
{
	int len = urlString.length();
	if (len == 0)
		return;

	// get query start
	int queryStart = urlString.indexOf('?');
	if (queryStart == -1) queryStart = len;

	// protocol
	int protocolEnd = urlString.indexOf(':');            //"://");

	if (protocolEnd != -1)
	{
		if ((len-protocolEnd > 3) && (urlString.substring(protocolEnd, protocolEnd + 3) == "://"))
		{
			Protocol = urlString.substring(0, protocolEnd);
			Protocol.toLowerCase();
			protocolEnd += 3;   //      Skip ://
		}
		else
		{
			protocolEnd = 0;  // no protocol
			Protocol = DEFAULT_URL_PROTOCOL;
		}
	}
	else
	{
		protocolEnd = 0;  // no protocol
		Protocol = DEFAULT_URL_PROTOCOL;
	}

	// host
	int hostStart = protocolEnd;
	int pathStart = urlString.indexOf('/', hostStart);  // get pathStart

	int portStart = urlString.indexOf(':', hostStart);  // check for port
	int portEnd = (pathStart != -1) ? pathStart : queryStart;
	bool hasPort = portStart != -1 && portStart < portEnd;
	int hostEnd = portEnd;
	if (hasPort)
		hostEnd = portStart;

	Host = urlString.substring(hostStart, hostEnd);
	//debugf("%d %d %s", hostStart, hostEnd, Host.c_str());

	// port
	if (hasPort)  // we have a port
	{
		portStart++;
		Port = urlString.substring(portStart, portEnd).toInt();
	}
	else
		Port = 80;

	// path
	if (pathStart != -1)
		Path = urlString.substring(pathStart, queryStart);

	// query
	if (queryStart != len)
		Query = urlString.substring(queryStart);
}
