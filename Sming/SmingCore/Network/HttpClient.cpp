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

HashMap<String, HttpConnection*> HttpClient::httpConnectionPool;
HashMap<String, RequestQueue*> HttpClient::queue;

#ifdef ENABLE_SSL
HashMap<String, SSLSessionId*> HttpClient::sslSessionIdPool;
#endif

/* Low Level Methods */
bool HttpClient::send(HttpRequest* request)
{
	String cacheKey = getCacheKey(request->uri);
	bool useSsl = (request->uri.Protocol == HTTPS_URL_PROTOCOL);

	if(!queue.contains(cacheKey)) {
		queue[cacheKey] = new RequestQueue;
	}

	if(!queue[cacheKey]->enqueue(request)) {
		// the queue is full and we cannot add more requests at the time.
		debug_e("The request queue is full at the moment");
		delete request;
		return false;
	}

	if(httpConnectionPool.contains(cacheKey) && httpConnectionPool[cacheKey]->getConnectionState() > eTCS_Connecting &&
	   !httpConnectionPool[cacheKey]->isActive()) {
		debug_d("Removing stale connection: State: %d, Active: %d",
				(int)httpConnectionPool[cacheKey]->getConnectionState(),
				(httpConnectionPool[cacheKey]->isActive() ? 1 : 0));
		delete httpConnectionPool[cacheKey];
		httpConnectionPool[cacheKey] = nullptr;
		httpConnectionPool.remove(cacheKey);
	}

	if(!httpConnectionPool.contains(cacheKey)) {
		debug_d("Creating new httpConnection");
		httpConnectionPool[cacheKey] = new HttpConnection(queue[cacheKey]);
	}

#ifdef ENABLE_SSL
	// Based on the URL decide if we should reuse the SSL and TCP pool
	if(useSsl) {
		if(!sslSessionIdPool.contains(cacheKey)) {
			sslSessionIdPool[cacheKey] = new SslSessionId;
		}
		httpConnectionPool[cacheKey]->addSslOptions(request->getSslOptions());
		httpConnectionPool[cacheKey]->pinCertificate(request->sslFingerprints);
		httpConnectionPool[cacheKey]->setSslKeyCert(request->sslKeyCertPair);
		httpConnectionPool[cacheKey]->sslSessionId = sslSessionIdPool[cacheKey];
	}
#endif

	return httpConnectionPool[cacheKey]->connect(request->uri.Host, request->uri.Port, useSsl);
}

// Convenience methods

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

	return send(request(url)->setResponseStream(fileStream)->setMethod(HTTP_GET)->onRequestComplete(requestComplete));
}

// end convenience methods

#ifdef ENABLE_SSL

void HttpClient::freeSslSessionPool()
{
	for(unsigned i = 0; i < sslSessionIdPool.count(); i++) {
		String key = sslSessionIdPool.keyAt(i);
		delete sslSessionIdPool[key];
		sslSessionIdPool[key] = nullptr;
	}
	sslSessionIdPool.clear();
}
#endif

void HttpClient::freeRequestQueue()
{
	for(unsigned i = 0; i < queue.count(); i++) {
		String key = queue.keyAt(i);
		RequestQueue* requestQueue = queue[key];
		HttpRequest* request = requestQueue->dequeue();
		while(request != nullptr) {
			delete request;
			request = requestQueue->dequeue();
		}
		queue[key]->flush();
		delete queue[key];
	}
	queue.clear();
}

void HttpClient::freeHttpConnectionPool()
{
	for(unsigned i = 0; i < httpConnectionPool.count(); i++) {
		String key = httpConnectionPool.keyAt(i);
		delete httpConnectionPool[key];
		httpConnectionPool[key] = nullptr;
		httpConnectionPool.remove(key);
	}
	httpConnectionPool.clear();
}

void HttpClient::cleanup()
{
#ifdef ENABLE_SSL
	freeSslSessionPool();
#endif
	freeHttpConnectionPool();
	freeRequestQueue();
}

HttpClient::~HttpClient()
{
	// DON'T call cleanup.
	// If you want to free all resources from HttpClients the correct sequence will be to
	// 1. Delete all instances of HttpClient
	// 2. Call the static method HttpClient::cleanup();
}

String HttpClient::getCacheKey(URL url)
{
	return url.Host + ':' + url.Port;
}
