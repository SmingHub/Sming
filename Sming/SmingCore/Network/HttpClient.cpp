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
		debugf("The request queue is full at the moment");
		delete request;
		return false;
	}

	if(httpConnectionPool.contains(cacheKey) &&
	   httpConnectionPool[cacheKey]->getConnectionState() > eTCS_Connecting &&
	   !httpConnectionPool[cacheKey]->isActive()
	) {

		debugf("Removing stale connection: State: %d, Active: %d", (int)httpConnectionPool[cacheKey]->getConnectionState(),
										(httpConnectionPool[cacheKey]->isActive() ? 1: 0));
		delete httpConnectionPool[cacheKey];
		httpConnectionPool[cacheKey] = NULL;
		httpConnectionPool.remove(cacheKey);
	}

	if(!httpConnectionPool.contains(cacheKey)) {
		debugf("Creating new httpConnection");
		httpConnectionPool[cacheKey] = new HttpConnection(queue[cacheKey]);
	}

#ifdef ENABLE_SSL
	// Based on the URL decide if we should reuse the SSL and TCP pool
	if(useSsl) {
		if (!sslSessionIdPool.contains(cacheKey)) {
			sslSessionIdPool[cacheKey] = (SSLSessionId *)malloc(sizeof(SSLSessionId));
			sslSessionIdPool[cacheKey]->value = NULL;
			sslSessionIdPool[cacheKey]->length = 0;
		}
		httpConnectionPool[cacheKey]->addSslOptions(request->getSslOptions());
		httpConnectionPool[cacheKey]->pinCertificate(request->sslFingerprint);
		httpConnectionPool[cacheKey]->setSslClientKeyCert(request->sslClientKeyCert);
		httpConnectionPool[cacheKey]->sslSessionId = sslSessionIdPool[cacheKey];
	}
#endif

	return httpConnectionPool[cacheKey]->connect(request->uri.Host, request->uri.Port, useSsl);
}

// Convenience methods

bool HttpClient::downloadString(const String& url, RequestCompletedDelegate requestComplete)
{
	return send(request(url)
				->setMethod(HTTP_GET)
				->onRequestComplete(requestComplete)
				);
}

bool HttpClient::downloadFile(const String& url, const String& saveFileName, RequestCompletedDelegate requestComplete /* = NULL */)
{
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

	FileOutputStream* fileStream = new FileOutputStream(file);

	return send(request(url)
				   ->setResponseStream(fileStream)
				   ->setMethod(HTTP_GET)
				   ->onRequestComplete(requestComplete)
			  );
}

// end convenience methods

HttpRequest* HttpClient::request(const String& url)
{
	return new HttpRequest(URL(url));
}

HashMap<String, HttpConnection *> HttpClient::httpConnectionPool;
HashMap<String, RequestQueue* > HttpClient::queue;

#ifdef ENABLE_SSL
HashMap<String, SSLSessionId* > HttpClient::sslSessionIdPool;

void HttpClient::freeSslSessionPool()
{
	for(int i=0; i< sslSessionIdPool.count(); i ++) {
		String key = sslSessionIdPool.keyAt(i);
		if(sslSessionIdPool[key]->value != NULL) {
			free(sslSessionIdPool[key]->value);
		}
		free(sslSessionIdPool[key]->value);
		free(sslSessionIdPool[key]);
	}
	sslSessionIdPool.clear();
}
#endif

void HttpClient::freeRequestQueue()
{
	for(int i=0; i< queue.count(); i ++) {
		String key = queue.keyAt(i);
		RequestQueue* requestQueue = queue[key];
		HttpRequest* request = requestQueue->dequeue();
		while(request != NULL){
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
	for(int i=0; i< httpConnectionPool.count(); i ++) {
		String key = httpConnectionPool.keyAt(i);
		delete httpConnectionPool[key];
		httpConnectionPool[key] = NULL;
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

}

String HttpClient::getCacheKey(URL url)
{
	return String(url.Host) + ":" + String(url.Port);
}
