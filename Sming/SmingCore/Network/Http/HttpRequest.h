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
#include "Data/Stream/DataSourceStream.h"
#include "Data/Stream/MultipartStream.h"
#include "Network/Http/HttpHeaders.h"

class HttpClient;
class HttpServerConnection;
class HttpConnection;

typedef Delegate<int(HttpConnection& client, HttpResponse& response)> RequestHeadersCompletedDelegate;
typedef Delegate<int(HttpConnection& client, const char* at, size_t length)> RequestBodyDelegate;
typedef Delegate<int(HttpConnection& client, bool successful)> RequestCompletedDelegate;

class HttpRequest
{
	friend class HttpClient;
	friend class HttpConnection;
	friend class HttpServerConnection;

public:
	HttpRequest(const URL& uri);
	HttpRequest(const HttpRequest& value);
	__forceinline HttpRequest* clone() const
	{
		return new HttpRequest(*this);
	}
	HttpRequest& operator=(const HttpRequest& rhs);
	~HttpRequest();

	HttpRequest* setURL(const URL& uri);

	HttpRequest* setMethod(const HttpMethod method);

	HttpRequest* setHeaders(const HttpHeaders& headers);

	HttpRequest* setHeader(const String& name, const String& value);

	HttpRequest* setPostParameters(const HttpParams& params);
	HttpRequest* setPostParameter(const String& name, const String& value);

	/**
	 * @brief Sets a file to be sent
	 * @param const String& name the name of the element in the form
	 * @param FileStream* stream - pointer to the file stream
	 *
	 * @return HttpRequest*
	 */
	HttpRequest* setFile(const String& name, FileStream* stream);

#ifdef ENABLE_HTTP_REQUEST_AUTH
	// Authentication adapters set here
	HttpRequest* setAuth(AuthAdapter* adapter);
#endif

	String getHeader(const String& name);

	String getPostParameter(const String& name);

	__forceinline String getPath()
	{
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
	 * @retval ReadWriteStream*
	 */
	ReadWriteStream* getBodyStream();

	HttpRequest* setBody(const String& body);
	HttpRequest* setBody(ReadWriteStream* stream);
	HttpRequest* setBody(uint8_t* rawData, size_t length);

	/**
	 * @brief Instead of storing the response body we can set a stream that will take care to process it
	 * @param ReadWriteStream *stream
	 *
	 * @retval HttpRequest*
	 */
	HttpRequest* setResponseStream(ReadWriteStream* stream);

	/**
	 * @brief Get access to the currently set response stream.
	 */
	ReadWriteStream* getResponseStream()
	{
		return responseStream;
	}

	HttpRequest* onHeadersComplete(RequestHeadersCompletedDelegate delegateFunction);
	HttpRequest* onBody(RequestBodyDelegate delegateFunction);
	HttpRequest* onRequestComplete(RequestCompletedDelegate delegateFunction);

	void reset();

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

	void* args = nullptr; // Used to store data that should be valid during a single request

protected:
	RequestHeadersCompletedDelegate headersCompletedDelegate;
	RequestBodyDelegate requestBodyDelegate;
	RequestCompletedDelegate requestCompletedDelegate;

	ReadWriteStream* stream = nullptr;
	ReadWriteStream* responseStream = nullptr;

#ifdef ENABLE_HTTP_REQUEST_AUTH
	AuthAdapter* auth = nullptr;
#endif

#ifdef ENABLE_SSL
	uint32_t sslOptions = 0;
	SSLFingerprints sslFingerprint;
	SSLKeyCertPair sslKeyCertPair;
#endif

private:
	HashMap<String, FileStream*> files;

	HttpParams* queryParams = nullptr; // << deprecated
};

#endif /* _SMING_CORE_HTTP_REQUEST_H_ */
