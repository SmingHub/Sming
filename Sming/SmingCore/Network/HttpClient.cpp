/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClient.cpp
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpClient.h"
#include "Data/Stream/FileStream.h"

ObjectMap<String, HttpClientConnection> HttpClient::httpConnectionPool;

bool HttpClient::send(HttpRequest* request)
{
	String cacheKey = getCacheKey(request->uri);

	auto connection = httpConnectionPool[cacheKey];

	if(connection != nullptr) {
		// Check existing connection
		if(connection->getConnectionState() > eTCS_Connecting && !connection->isActive()) {
			debug_d("Removing stale connection: State: %d, Active: %d", connection->getConnectionState(),
					connection->isActive());
			connection = nullptr;
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
	}

	return connection->send(request);
}

bool HttpClient::downloadFile(const String& url, const String& saveFileName, RequestCompletedDelegate requestComplete)
{
	URL uri(url);

	String file;
	if(saveFileName.length() == 0) {
		file = uri.Path;
		int p = file.lastIndexOf('/');
		if(p >= 0) {
			file.remove(0, p + 1);
		}
	} else {
		file = saveFileName;
	}

	auto fileStream = new FileStream();
	if(!fileStream->open(file, eFO_CreateNewAlways | eFO_WriteOnly)) {
		debug_e("HttpClient failed to open \"%s\"", file.c_str());
		return false;
	}

	return send(
		createRequest(url)->setResponseStream(fileStream)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
}
