/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpRequest.h
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#pragma once

#include "HttpCommon.h"
#ifdef ENABLE_HTTP_REQUEST_AUTH
#include "HttpRequestAuth.h"
#endif
#include "../TcpConnection.h"
#include "Data/Stream/DataSourceStream.h"
#include "HttpHeaders.h"
#include "HttpParams.h"
#include "Data/ObjectMap.h"

class HttpConnection;

using RequestHeadersCompletedDelegate = Delegate<int(HttpConnection& client, HttpResponse& response)>;
using RequestBodyDelegate = Delegate<int(HttpConnection& client, const char* at, size_t length)>;
using RequestCompletedDelegate = Delegate<int(HttpConnection& client, bool successful)>;

/**
 * @brief Encapsulates an incoming or outgoing request
 * @ingroup http
 *
 */
class HttpRequest
{
	friend class HttpClientConnection;
	friend class HttpServerConnection;

public:
	HttpRequest()
	{
	}

	HttpRequest(const Url& uri) : uri(uri)
	{
	}

	/**
	 * @brief Copy constructor
	 * @note Internal streams are not copied so these must be dealt with afterwards
	 */
	HttpRequest(const HttpRequest& value)
		: uri(value.uri), method(value.method), headers(value.headers), postParams(value.postParams),
		  headersCompletedDelegate(value.headersCompletedDelegate), requestBodyDelegate(value.requestBodyDelegate),
		  requestCompletedDelegate(value.requestCompletedDelegate), sslInitDelegate(value.sslInitDelegate)
	{
	}

	/**
	 * @brief Clone this request into a new object using the copy constructor
	 * @retval HttpRequest* The new request object
	 * @see HttpRequest(const HttpRequest& value)
	 */
	HttpRequest* clone() const
	{
		return new HttpRequest(*this);
	}

	/** @deprecated Please use `clone()` instead */
	HttpRequest& operator=(const HttpRequest& rhs) SMING_DEPRECATED
	{
		return *this;
	}

	~HttpRequest()
	{
		reset();
	}

	HttpRequest* setURL(const Url& uri)
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
	 * @deprecated Set postParams directly, i.e. `request.postParams = params`
	 */
	HttpRequest* setPostParameters(const HttpParams& params) SMING_DEPRECATED
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
	 * @param formElementName The name of the element in the form
	 * @param stream Pointer to the stream (doesn't have to be a FileStream)
	 *
	 * @retval HttpRequest*
	 */
	HttpRequest* setFile(const String& formElementName, ReadWriteStream* stream)
	{
		if(stream != nullptr) {
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

	/** @deprecated Use `uri.Path` instead */
	String getPath() SMING_DEPRECATED
	{
		return uri.Path;
	}

	/* @deprecated Use methods of `uri.Query` instead */
	String getQueryParameter(const String& parameterName, const String& defaultValue = nullptr) const
	{
		return reinterpret_cast<const HttpParams&>(uri.Query)[parameterName] ?: defaultValue;
	}

	/**
	 * @brief Moves content from the body stream into a String.
	 * @retval String
	 * @note Move semantics are used to ensure that no/minimal additional memory is required.
	 * If your application has set a non-memory stream type then the method will
	 * fail and return an invalid String. The stream content will be left unchanged.
	 */
	String getBody()
	{
		String s;
		if(bodyStream != nullptr) {
			bodyStream->moveString(s);
		}
		return s;
	}

	/**
	 * @brief Return the current body stream
	 * @retval IDataSourceStream*
	 * @note may return null
	 */
	IDataSourceStream* getBodyStream()
	{
		return bodyStream;
	}

	/**
	 * @name Set request body content
	 * @{
	 */
	HttpRequest* setBody(const String& body)
	{
		return setBody(reinterpret_cast<const uint8_t*>(body.c_str()), body.length());
	}

	HttpRequest* setBody(String&& body) noexcept;

	HttpRequest* setBody(IDataSourceStream* stream);

	HttpRequest* setBody(const uint8_t* rawData, size_t length);
	/** @} */

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

	/**
	 * @brief Callback delegate type used to initialise an SSL session for a given request
	 */
	using SslInitDelegate = Delegate<void(Ssl::Session& session, HttpRequest& request)>;

	/**
	 * @brief To customise SSL session options, provide a callback
	 * @param delegate Invoked before creating SSL connection
	 */
	HttpRequest* onSslInit(SslInitDelegate delegate)
	{
		sslInitDelegate = delegate;
		return this;
	}

	/**
	 * @brief Tries to present a readable version of the current request values
	 * @retval String
	 */
	String toString() const;

	/**
	 * @brief Tries to present a readable version of the request
	 * @param req
	 *
	 * @retval String
	 */
	static String toString(const HttpRequest& req)
	{
		return req.toString();
	}

public:
	Url uri;
	HttpMethod method = HTTP_GET;
	HttpHeaders headers;
	HttpParams postParams;
	HttpFiles files;

	int retries = 0; // how many times the request should be send again...

	void* args = nullptr; // Used to store data that should be valid during a single request

protected:
	RequestHeadersCompletedDelegate headersCompletedDelegate;
	RequestBodyDelegate requestBodyDelegate;
	RequestCompletedDelegate requestCompletedDelegate;
	SslInitDelegate sslInitDelegate;

	IDataSourceStream* bodyStream = nullptr;
	ReadWriteStream* responseStream = nullptr; ///< User-requested stream to store response

#ifdef ENABLE_HTTP_REQUEST_AUTH
	AuthAdapter* auth = nullptr;
#endif

private:
	HttpParams* queryParams = nullptr; // << @todo deprecate
};

inline String toString(const HttpRequest& req)
{
	return req.toString();
}
