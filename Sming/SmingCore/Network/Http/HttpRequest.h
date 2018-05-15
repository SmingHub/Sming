/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 *
 * HttpRequest
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_HTTP_REQUEST_H_
#define _SMING_CORE_HTTP_REQUEST_H_

#include "HttpCommon.h"
#ifdef ENABLE_HTTP_REQUEST_AUTH
#include "HttpRequestAuth.h"
#endif
#include "../TcpConnection.h"
#include "Data/Stream/OutputStream.h"

class HttpClient;
class HttpServerConnection;
class HttpConnection;

typedef Delegate<int(HttpConnection& client, HttpHeaders& headers)> RequestHeadersCompletedDelegate;
typedef Delegate<int(HttpConnection& client, const char *at, size_t length)> RequestBodyDelegate;
typedef Delegate<int(HttpConnection& client, bool successful)> RequestCompletedDelegate;

class HttpRequest {
	friend class HttpClient;
	friend class HttpConnection;
	friend class HttpServerConnection;

public:

	HttpRequest(const URL& uri);
	HttpRequest(const HttpRequest& value);
	__forceinline HttpRequest* clone() const { return new HttpRequest(*this); }
	HttpRequest& operator = (const HttpRequest& rhs);
	~HttpRequest();

	HttpRequest* setURL(const URL& uri);

	HttpRequest* setMethod(const HttpMethod method);

	HttpRequest* setHeaders(const HttpHeaders& headers);

	HttpRequest* setHeader(const String& name, const String& value);

	HttpRequest* setPostParameters(const HttpParams& params);
	HttpRequest* setPostParameter(const String& name, const String& value);


#ifdef ENABLE_HTTP_REQUEST_AUTH
	// Authentication adapters set here
	HttpRequest* setAuth(AuthAdapter *adapter);
#endif

	String getHeader(const String& name);

	String getPostParameter(const String& name);

	__forceinline String getPath() {
		return uri.Path;
	}

	String getQueryParameter(const String& parameterName, const String& defaultValue = "");

	/**
	 * @brief Returns content from the body stream as string.
	 * @retval String
	 *
	 * @note This method consumes the stream and it will work only with text data.
	 * 		 If you have binary data in the stream use getBodyStream instead.
	 */
	String getBody();

	/**
	 * @brief Returns pointer to the current body stream
	 * @retval IDataSourceStream*
	 */
	IDataSourceStream* getBodyStream();

#ifdef ENABLE_SSL
 	HttpRequest* setSslOptions(uint32_t sslOptions);
 	uint32_t getSslOptions();

 	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 * 			Check if SHA256 hash of Subject Public Key Info matches the one given.
	 * @param SSLFingerprints - passes the certificate fingerprints by reference.
	 *
	 * @return bool  true of success, false or failure
	 */
 	HttpRequest* pinCertificate(const SSLFingerprints& fingerprints);

 	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @param SSLKeyCertPair
	 * @param bool freeAfterHandshake
	 *
	 * @return HttpRequest pointer
	 */
 	HttpRequest* setSslKeyCert(const SSLKeyCertPair& keyCertPair);
#endif

	HttpRequest* setBody(const String& body);
	HttpRequest* setBody(ReadWriteStream *stream);
	HttpRequest* setBody(uint8_t *rawData, size_t length);

	HttpRequest* setResponseStream(IOutputStream *stream);

	HttpRequest* onHeadersComplete(RequestHeadersCompletedDelegate delegateFunction);
	HttpRequest* onBody(RequestBodyDelegate delegateFunction);
	HttpRequest* onRequestComplete(RequestCompletedDelegate delegateFunction);

	void reset();

#ifndef SMING_RELEASE
	/**
	 * @brief Tries to present a readable version of the current request values
	 * @return String
	 */
	String toString();
#endif

public:
	URL uri;
	HttpMethod method = HTTP_GET;
	HttpHeaders headers;

	HttpParams postParams;

	int retries = 0; // how many times the request should be send again...

	void *args = NULL; // Used to store data that should be valid during a single request

protected:
	RequestHeadersCompletedDelegate headersCompletedDelegate;
	RequestBodyDelegate requestBodyDelegate;
	RequestCompletedDelegate requestCompletedDelegate;

	uint8_t *rawData = NULL;
	size_t rawDataLength = 0;
	ReadWriteStream *stream = NULL;

	IOutputStream *responseStream = NULL;

#ifdef ENABLE_HTTP_REQUEST_AUTH
	AuthAdapter *auth = NULL;
#endif

#ifdef ENABLE_SSL
	uint32_t sslOptions = 0;
	SSLFingerprints sslFingerprint;
	SSLKeyCertPair  sslKeyCertPair;
#endif

private:
	HttpParams* queryParams = NULL; // << deprecated
};

#endif /* _SMING_CORE_HTTP_REQUEST_H_ */
