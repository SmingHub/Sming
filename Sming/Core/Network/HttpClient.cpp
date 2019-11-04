/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClient.cpp
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpClient.h"
#include "Data/Stream/FileStream.h"

HttpClient::HttpConnectionPool HttpClient::httpConnectionPool;

bool HttpClient::send(HttpRequest* request)
{
	String cacheKey = getCacheKey(request->uri);

	HttpClientConnection* connection = nullptr;

	int i = httpConnectionPool.indexOf(cacheKey);
	if(i >= 0) {
		// Check existing connection
		connection = httpConnectionPool.valueAt(i);
		if(connection->getConnectionState() > eTCS_Connecting && !connection->isActive()) {
			debug_d("Removing stale connection: State: %d, Active: %d", connection->getConnectionState(),
					connection->isActive());
			delete connection;
			connection = nullptr;
			httpConnectionPool.removeAt(i);
		}
	}

	if(connection == nullptr) {
		debug_d("Creating new HttpClientConnection");
		connection = new HttpClientConnection();
		if(connection == nullptr) {
			// Out of memory
			delete request;
			return false;
		}
		httpConnectionPool[cacheKey] = connection;
	}

	return connection->send(request);
}

bool HttpClient::downloadFile(const Url& url, const String& saveFileName, RequestCompletedDelegate requestComplete)
{
	String file = saveFileName;
	if(file.length() == 0) {
		file = url.getFileName();
	}

	auto fileStream = new FileStream();
	if(!fileStream->open(file, eFO_CreateNewAlways | eFO_WriteOnly)) {
		debug_e("HttpClient failed to open \"%s\"", file.c_str());
		delete fileStream;
		return false;
	}

	return send(
		createRequest(url)->setResponseStream(fileStream)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
}
