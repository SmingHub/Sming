/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpClientConnection.cpp
 *
 ****/

#include "HttpClientConnection.h"
#include "Data/Stream/FileStream.h"

HttpClientConnection::~HttpClientConnection()
{
	// ObjectQueue doesn't own its objects
	while(requestQueue.count() != 0) {
		delete requestQueue.dequeue();
	}
#ifdef ENABLE_SSL
	delete sslSessionId;
#endif
}

bool HttpClientConnection::send(HttpRequest* request)
{
	if(!requestQueue.enqueue(request)) {
		// the queue is full and we cannot add more requests at the time.
		debug_e("The request queue is full at the moment");
		delete request;
		return false;
	}

	bool useSsl = (request->uri.Protocol == HTTPS_URL_PROTOCOL);

#ifdef ENABLE_SSL
	// Based on the URL decide if we should reuse the SSL and TCP pool
	if(useSsl) {
		if(sslSessionId == nullptr) {
			sslSessionId = new SslSessionId;
		}
		addSslOptions(request->getSslOptions());
		pinCertificate(request->sslFingerprints);
		setSslKeyCert(request->sslKeyCertPair);
	}
#endif

	return connect(request->uri.Host, request->uri.Port, useSsl);
}
