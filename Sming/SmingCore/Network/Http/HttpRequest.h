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
#include "HttpHeaders.h"
#include "HttpParams.h"

class HttpClient;
class HttpServerConnection;
class HttpConnection;

typedef Delegate<int(HttpConnection& client, HttpResponse& response)> RequestHeadersCompletedDelegate;
typedef Delegate<int(HttpConnection& client, const char* at, size_t length)> RequestBodyDelegate;
typedef Delegate<int(HttpConnection& client, bool successful)> RequestCompletedDelegate;

/*
 * Encapsulates an incoming or outgoing request
 */
class HttpRequest
{
	friend class HttpConnection;
	friend class HttpClientConnection;
	friend class HttpServerConnection;

public:
	HttpRequest()
	{
	}

	HttpRequest(const URL& uri) : uri(uri)
	{
	}

	HttpRequest(const HttpRequest& value);

	HttpRequest* clone() const
	{
		return new HttpRequest(*this);
	}

	HttpRequest& operator=(const HttpRequest& rhs);

	~HttpRequest()
	{
		reset();
	}

	HttpRequest* setURL(const URL& uri)
	{
		this->uri = uri;
		return this;
	}

	HttpRequest* setMethod(HttpMethod method)
	{
		this->method = method;
		return this;
	}

	HttpRequest* setHeaders(const HttpHeaders& headers)
	{
		this->headers.setMultiple(headers);
		return this;
	}

	HttpRequest* setHeader(const String& name, const String& value)
	{
		headers[name] = value;
		return this;
	}

	/**
	 * @deprecated This method is deprecated and will be removed in the coming versions.
	 * 			   Please set postParams directly, i.e. request.postParams = params
	 */
	HttpRequest* setPostParameters(const HttpParams& params) __deprecated
	{
		postParams = params;
		return this;
	}

	HttpRequest* setPostParameter(const String& name, const String& value)
	{
		postParams[name] = value;
		return this;
	}

	/**
	 * @brief Sets a file to be sent
	 * @param const String& formElementName the name of the element in the form
	 * @param IDataSourceStream* stream - pointer to the stream (doesn't have to be a FileStream)
	 *
	 * @return HttpRequest*
	 */
	HttpRequest* setFile(const String& formElementName, IDataSourceStream* stream)
	{
		if(stream) {
			files[formElementName] = stream;
		}
		return this;
	}

#ifdef ENABLE_HTTP_REQUEST_AUTH
	// Authentication adapters set here
	HttpRequest* setAuth(AuthAdapter* adapter)
	{
		adapter->setRequest(this);
		auth = adapter;
		return this;
	}
#endif

	const String& getHeader(const String& name)
	{
		return static_cast<const HttpHeaders&>(headers)[name];
	}

	const String& getPostParameter(const String& name)
	{
		return static_cast<const HttpParams&>(postParams)[name];
	}

	/* @deprecated use uri.Path */
	String getPath() __deprecated
	{
		return uri.Path;
	}

	/* @todo deprecate: use uri methods */
	String getQueryParameter(const String& parameterName, const String& defaultValue = nullptr);

	/**
	 * @brief Returns content from the body stream as string.
	 * @retval String
	 *
	 * @note This method consumes the stream and it will work only with text data.
	 * 		 If you have binary data in the stream use getBodyStream instead.
	 *
	 * @note Allocation of String doubles amount of memory required, so use with care.
	 */
	String getBody();

	/**
	 * @brief Return the current body stream and pass ownership to the caller
	 * @retval IDataSourceStream*
	 * @note may return null
	 */
	IDataSourceStream* getBodyStream();

	HttpRequest* setBody(const String& body)
	{
		setBody((uint8_t*)body.c_str(), body.length());
		return this;
	}

	HttpRequest* setBody(IDataSourceStream* stream);

	HttpRequest* setBody(uint8_t* rawData, size_t length);

	/**
	 * @brief Instead of storing the response body we can set a stream that will take care to process it
	 * @param stream
	 * @retval HttpRequest*
	 *
	 * @note The response to this request will be stored in the user-provided stream.
	 */
	HttpRequest* setResponseStream(ReadWriteStream* stream);

	/**
	 * @brief Get the response stream (if any)
	 */
	ReadWriteStream* getResponseStream()
	{
		return responseStream;
	}

	HttpRequest* onHeadersComplete(RequestHeadersCompletedDelegate delegateFunction)
	{
		headersCompletedDelegate = delegateFunction;
		return this;
	}

	HttpRequest* onBody(RequestBodyDelegate delegateFunction)
	{
		requestBodyDelegate = delegateFunction;
		return this;
	}

	HttpRequest* onRequestComplete(RequestCompletedDelegate delegateFunction)
	{
		requestCompletedDelegate = delegateFunction;
		return this;
	}

	/** @brief Clear buffers and reset to default state in preparation for another request */
	void reset();

#ifdef ENABLE_SSL
	HttpRequest* setSslOptions(uint32_t sslOptions)
	{
		this->sslOptions = sslOptions;
		return this;
	}

	uint32_t getSslOptions()
	{
		return sslOptions;
	}

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 * 			Check if SHA256 hash of Subject Public Key Info matches the one given.
	 * @param	fingerprints - passes the certificate fingerprints by reference.
	 *
	 * @return bool  true of success, false or failure
	 */
	HttpRequest* pinCertificate(SslFingerprints& fingerprints)
	{
		sslFingerprints = fingerprints;
		return this;
	}

	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @param SSLKeyCertPair
	 * @param bool freeAfterHandshake
	 *
	 * @return HttpRequest pointer
	 */
	HttpRequest* setSslKeyCert(const SslKeyCertPair& keyCertPair)
	{
		sslKeyCertPair = keyCertPair;
		return this;
	}
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

	IDataSourceStream* bodyStream = nullptr;
	ReadWriteStream* responseStream = nullptr; ///< User-requested stream to store response

#ifdef ENABLE_HTTP_REQUEST_AUTH
	AuthAdapter* auth = nullptr;
#endif

#ifdef ENABLE_SSL
	uint32_t sslOptions = 0;
	SslFingerprints sslFingerprints;
	SslKeyCertPair sslKeyCertPair;
#endif

private:
	HashMap<String, IDataSourceStream*> files;

	HttpParams* queryParams = nullptr; // << @todo deprecate
};

#endif /* _SMING_CORE_HTTP_REQUEST_H_ */
