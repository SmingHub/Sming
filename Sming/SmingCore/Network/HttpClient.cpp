/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClient
 *
 * Modified: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpClient.h"
#include "Data/Stream/FileStream.h"


/* 13/8/2018 (mikee47)
 *
 * These two maps redefined with the new ObjectMap class which owns all the
 * contained objects; simplifies code
 *
 */

ObjectMap<HttpConnection> HttpClient::_connectionPool;
ObjectMap<RequestQueue> HttpClient::_requestQueueMap;
#ifdef ENABLE_SSL
ObjectMap<SSLSessionId> HttpClient::_sslSessionIdPool;
#endif

/* Low Level Methods */
bool HttpClient::send(HttpRequest* request)
{
	String cacheKey = getCacheKey(request->uri);
	bool useSsl = (request->uri.protocol() == HTTPS_URL_PROTOCOL);

	auto& queue = _requestQueueMap[cacheKey];
	if (!queue) {
		queue = new RequestQueue;
		if (!queue)
			return false;
	}

	if (!queue->enqueue(request)) {
		// the queue is full and we cannot add more requests at the time.
		debug_e("The request queue is full at the moment");
		delete request;
		return false;
	}

	auto& conn = _connectionPool[cacheKey];
	if (conn && conn->getConnectionState() > eTCS_Connecting && !conn->isActive()) {
		debug_d("Removing stale connection: State: %d", conn->getConnectionState());
		delete conn;
		conn = nullptr;
	}

	if (!conn) {
		debug_d("Creating new httpConnection");
		conn = new HttpConnection(queue);
		if (!conn)
			return false;
	}

#ifdef ENABLE_SSL
	// Based on the URL decide if we should reuse the SSL and TCP pool
	if (useSsl) {
		auto& sessionId = _sslSessionIdPool[cacheKey];
		if (!sessionId)
			sessionId = new SSLSessionId;
		conn->addSslOptions(request->getSslOptions());
		conn->pinCertificate(request->getCertificate());
		conn->setSslKeyCert(request->getSslKeyCert());
		conn->setSslSessionId(sessionId);
	}
#endif

	return conn->connect(request->uri.host(), request->uri.port(), useSsl);
}

// Convenience methods

bool HttpClient::downloadString(const String& url, RequestCompletedDelegate requestComplete)
{
	auto req = createRequest(url);
	req->setMethod(HTTP_GET);
	req->onRequestComplete(requestComplete);
	return send(req);
}

bool HttpClient::downloadFile(const String& url, const String& saveFileName,
							  RequestCompletedDelegate requestComplete /* = nullptr */)
{
	URL uri = URL(url);

	String file;
	if (saveFileName.length() == 0)
		file = uri.fileName();
	else
		file = saveFileName;

	FileStream* fileStream = new FileStream(file, eFO_CreateNewAlways | eFO_WriteOnly);

	auto req = createRequest(url);
	req->setResponseStream(fileStream);
	req->setMethod(HTTP_GET);
	req->onRequestComplete(requestComplete);
	return send(req);
}

// end convenience methods

void HttpClient::cleanup()
{
#ifdef ENABLE_SSL
	_sslSessionIdPool.clear();
#endif
	_connectionPool.clear();
	_requestQueueMap.clear();
}

HttpClient::~HttpClient()
{
	// DON'T call cleanup.
	// If you want to free all resources from HttpClients the correct sequence will be to
	// 1. Delete all instances of HttpClient
	// 2. Call the static method HttpClient::cleanup();
}

String HttpClient::getCacheKey(const URL& url)
{
	return url.host() + ':' + url.port();
}
