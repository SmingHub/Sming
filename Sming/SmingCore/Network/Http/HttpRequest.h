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

/*
 * 13/8/2018 (mikee47)
 *
 * 	friend classes removed; instead we have methods to deal with
 */

#ifndef _SMING_CORE_HTTP_REQUEST_H_
#define _SMING_CORE_HTTP_REQUEST_H_

#include "HttpCommon.h"
#ifdef ENABLE_HTTP_REQUEST_AUTH
#include "HttpRequestAuth.h"
#endif
#include "../TcpConnection.h"
#include "Data/Stream/DataSourceStream.h"
#include "Data/Stream/MultipartStream.h"
#include "Data/HttpHeaders.h"
#include "Data/HttpParams.h"
#include "ObjectMap.h"

class HttpConnection;

typedef Delegate<int(HttpConnection& client, HttpResponse& response)> RequestHeadersCompletedDelegate;
typedef Delegate<int(HttpConnection& client, const char* at, size_t length)> RequestBodyDelegate;
typedef Delegate<int(HttpConnection& client, bool successful)> RequestCompletedDelegate;

/*
 * Encapsulates an incoming or outgoing request
 */
class HttpRequest {
public:
	HttpRequest()
	{}

	HttpRequest(const URL& uri) : uri(uri)
	{}

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

	HttpRequest* setMethod(const HttpMethod method)
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

	/** @deprecated this replaces existing post parameter set, which is different to
	 * behaviour of setHeaders which only replaces specified values.
	 * Better to use appropriate method of postParams.
	 *
	 */
	HttpRequest* setPostParameter(const HttpParams& params)
	{
		postParams = params;
		return this;
	}

	HttpRequest* setPostParameter(const String& name, const String& value)
	{
		postParams[name] = value;
		return this;
	}

	String methodStr()
	{
		return http_method_str(method);
	}

	/**
	 * @brief Sets a file to be sent
	 * @param const String& name the name of the element in the form
	 * @param FileStream* stream - pointer to the file stream
	 *
	 * @return HttpRequest*
	 */
	HttpRequest* setFile(const String& name, IDataSourceStream* stream)
	{
		if (stream)
			_files[name] = stream;
		return this;
	}

#ifdef ENABLE_HTTP_REQUEST_AUTH
	// Authentication adapters set here
	HttpRequest* setAuth(AuthAdapter* adapter)
	{
		adapter->setRequest(this);
		_auth = adapter;
		return this;
	}
#endif

	/* @deprecated use headers */
	const String& getHeader(const String& name)
	{
		return headers[name];
	}

	/* @deprecated use postParams */
	const String& getPostParameter(const String& name)
	{
		return postParams[name];
	}

	/* @deprecated  use uri methods */
	String getPath()
	{
		return uri.path();
	}

	/* @deprecated  use uri methods */
	const String& getQueryParameter(const String& parameterName, const String& defaultValue = nullptr)
	{
		return uri.QueryParameters()[parameterName] ?: defaultValue;
	}

	/**
	 * @brief Returns content from the body stream as string.
	 * 	@retval String
	 *
	 * 	@note This method consumes the stream and it will work only with text data.
	 * 		 If you have binary data in the stream use getBodyStream instead.
	 */
	String getBody();

	/** @brief Get the HTTP request line with line ending
	 *  @note This is the first line in the request headers
	 */
	String getRequestLine();

	/*
	 * Prepare headers reading for sending
	 */
	HttpHeaders& prepareHeaders();

	/**
	 * @brief Return the current body stream and pass ownership to the caller
	 * @retval IDataSourceStream*
	 * @note may return null
	 */
	IDataSourceStream* getBodyStream();

	bool hasBody()
	{
		return _bodyStream != nullptr;
	}

	HttpRequest* setBody(const String& body)
	{
		setBody((uint8_t*)body.c_str(), body.length());
		return this;
	}

	HttpRequest* setBody(IDataSourceStream* stream);

	HttpRequest* setBody(uint8_t* rawData, size_t length);

	/**
	 * @brief Instead of storing the response body we can set a stream that will take care to process it
	 * @param ReadWriteStream *stream
	 *
	 * @retval HttpRequest*
	 *
	 * In other words, the response to this message will be stored in the user-provided stream.
	 */
	HttpRequest* setResponseStream(ReadWriteStream* stream);

	/**
	 * @brief Get the response stream (if any) and pass ownership to the caller
	 */
	ReadWriteStream* getResponseStream()
	{
		auto s = _responseStream;
		_responseStream = nullptr;
		return s;
	}

	HttpRequest* onHeadersComplete(RequestHeadersCompletedDelegate delegateFunction)
	{
		_headersCompletedDelegate = delegateFunction;
		return this;
	}

	HttpRequest* onBody(RequestBodyDelegate delegateFunction)
	{
		_requestBodyDelegate = delegateFunction;
		return this;
	}

	HttpRequest* onRequestComplete(RequestCompletedDelegate delegateFunction)
	{
		_requestCompletedDelegate = delegateFunction;
		return this;
	}

	/*
	 * Clear buffers and reset to default state in preparation for another request
	 */
	void reset();

#ifdef ENABLE_SSL
	HttpRequest* setSslOptions(uint32_t sslOptions)
	{
		_sslOptions = sslOptions;
		return this;
	}

	uint32_t getSslOptions()
	{
		return _sslOptions;
	}

	/**
	 * @brief   Requires(pins) the remote SSL certificate to match certain fingerprints
	 * 			Check if SHA256 hash of Subject Public Key Info matches the one given.
	 * @param SSLFingerprints - passes the certificate fingerprints by reference.
	 *
	 * @return bool  true of success, false or failure
	 */
	HttpRequest* pinCertificate(const SSLFingerprints& fingerprints)
	{
		_sslFingerprint = fingerprints;
		return this;
	}

	/**
 	 * @brief Retrieves certificate fingerprint, passing ownership to caller.
 	 */
	SSLFingerprints getCertificate()
	{
		auto ret = _sslFingerprint;
		memset(&_sslFingerprint, 0, sizeof(_sslFingerprint));
		return ret;
	}

	/**
	 * @brief Sets client private key, certificate and password from memory
	 * @param SSLKeyCertPair
	 * @param bool freeAfterHandshake
	 *
	 * @return HttpRequest pointer
	 */
	HttpRequest* setSslKeyCert(const SSLKeyCertPair& keyCertPair)
	{
		_sslKeyCertPair = keyCertPair;
		return this;
	}

	/**
 	 * @brief Retrieves certificate pair, passing ownership to caller.
 	 */
	SSLKeyCertPair getSslKeyCert()
	{
		auto ret = _sslKeyCertPair;
		memset(&_sslKeyCertPair, 0, sizeof(_sslKeyCertPair));
		return ret;
	}

#endif

#ifndef SMING_RELEASE
	/**
	 * @brief Tries to present a readable version of the current request values
	 * @return String
	 */
	String toString();
#endif

	/* These methods are called during request execution */

	int RequestHeadersCompleted(HttpConnection& client, HttpResponse& response)
	{
		if (_auth)
			_auth->setResponse(&response);

		return _headersCompletedDelegate ? _headersCompletedDelegate(client, response) : 0;
	}

	int RequestBody(HttpConnection& client, const char* at, size_t length)
	{
		return _requestBodyDelegate ? _requestBodyDelegate(client, at, length) : 0;
	}

	int RequestCompleted(HttpConnection& client, bool successful)
	{
		return _requestCompletedDelegate ? _requestCompletedDelegate(client, successful) : 0;
	}

public:
	URL uri;
	HttpMethod method = HTTP_GET;
	HttpHeaders headers;
	HttpParams postParams;
	// how many times the request should be send again...
	int retries = 0;
	// Used to store data that should be valid during a single request
	void* args = nullptr;

private:
	HttpPartResult multipartProducer();

protected:
	RequestHeadersCompletedDelegate _headersCompletedDelegate;
	RequestBodyDelegate _requestBodyDelegate;
	RequestCompletedDelegate _requestCompletedDelegate;

	IDataSourceStream* _bodyStream = nullptr;
	// User-requested stream to store response
	ReadWriteStream* _responseStream = nullptr;

#ifdef ENABLE_HTTP_REQUEST_AUTH
	AuthAdapter* _auth = nullptr;
#endif

#ifdef ENABLE_SSL
	uint32_t _sslOptions = 0;
	SSLFingerprints _sslFingerprint;
	SSLKeyCertPair _sslKeyCertPair;
#endif

private:
	ObjectMap<IDataSourceStream> _files;
};

#endif /* _SMING_CORE_HTTP_REQUEST_H_ */
