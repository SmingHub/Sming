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

#include "HttpRequest.h"

HttpRequest::HttpRequest(const URL& uri) : uri(uri)
{}

HttpRequest::HttpRequest(const HttpRequest& value) : uri(value.uri)
{
	*this = value;
	method = value.method;
	if (value.headers.count()) {
		setHeaders(value.headers);
	}
	headersCompletedDelegate = value.headersCompletedDelegate;
	requestBodyDelegate = value.requestBodyDelegate;
	requestCompletedDelegate = value.requestCompletedDelegate;

	debug_w("Warning: HttpRequest streams are not copied..");

#ifdef ENABLE_SSL
	sslOptions = value.sslOptions;
	sslFingerprint = value.sslFingerprint;
	sslKeyCertPair = value.sslKeyCertPair;
#endif
}

HttpRequest& HttpRequest::operator=(const HttpRequest& rhs)
{
	if (this == &rhs)
		return *this;

	// TODO: FIX this...
	//	if (rhs.buffer) copy(rhs.buffer, rhs.len);
	//	else invalidate();

	return *this;
}

HttpRequest::~HttpRequest()
{
	reset();
}

HttpRequest* HttpRequest::setURL(const URL& uri)
{
	this->uri = uri;
	return this;
}

HttpRequest* HttpRequest::setMethod(const HttpMethod method)
{
	this->method = method;
	return this;
}

HttpRequest* HttpRequest::setHeaders(const HttpHeaders& headers)
{
	for (int i = 0; i < headers.count(); i++) {
		this->headers[headers.keyAt(i)] = headers.valueAt(i);
	}
	return this;
}

HttpRequest* HttpRequest::setHeader(const String& name, const String& value)
{
	this->headers[name] = value;
	return this;
}

HttpRequest* HttpRequest::setPostParameters(const HttpParams& params)
{
	postParams = params;
	return this;
}

HttpRequest* HttpRequest::setPostParameter(const String& name, const String& value)
{
	postParams[name] = value;
	return this;
}

HttpRequest* HttpRequest::setFile(const String& formElementName, FileStream* stream)
{
	if (stream == null) {
		return this;
	}

	files[formElementName] = stream;

	return this;
}

#ifdef ENABLE_HTTP_REQUEST_AUTH
HttpRequest* HttpRequest::setAuth(AuthAdapter* adapter)
{
	adapter->setRequest(this);
	auth = adapter;
	return this;
}
#endif

String HttpRequest::getHeader(const String& name)
{
	if (!headers.contains(name)) {
		return "";
	}

	return headers[name];
}

String HttpRequest::getPostParameter(const String& name)
{
	if (!postParams.contains(name)) {
		return "";
	}

	return postParams[name];
}

String HttpRequest::getQueryParameter(const String& parameterName, const String& defaultValue /* = "" */)
{
	if (queryParams == NULL) {
		queryParams = new HttpParams();
		if (!uri.Query.length()) {
			return defaultValue;
		}

		String query = uri.Query.substring(1);
		Vector<String> parts;
		splitString(query, '&', parts);
		for (int i = 0; i < parts.count(); i++) {
			Vector<String> pair;
			int count = splitString(parts[i], '=', pair);
			if (count != 2) {
				debug_w("getQueryParameter: Missing = in query string: %s", parts[i].c_str());
				continue;
			}
			(*queryParams)[pair.at(0)] = pair.at(1); // TODO: name and value URI decoding...
		}
	}

	if (queryParams->contains(parameterName)) {
		return (*queryParams)[parameterName];
	}

	return defaultValue;
}

String HttpRequest::getBody()
{
	if (stream == NULL) {
		return "";
	}

	String ret;
	if (stream->available() != -1 && stream->getStreamType() == eSST_Memory) {
		MemoryDataStream* memory = (MemoryDataStream*)stream;
		char buf[1024];
		while (stream->available() > 0) {
			int available = memory->readMemoryBlock(buf, 1024);
			memory->seek(available);
			ret += String(buf, available);
			if (available < 1024) {
				break;
			}
		}
	}
	return ret;
}

ReadWriteStream* HttpRequest::getBodyStream()
{
	return stream;
}

HttpRequest* HttpRequest::setResponseStream(ReadWriteStream* stream)
{
	if (responseStream != NULL) {
		debug_e("HttpRequest::setResponseStream: Discarding already set stream!");
		delete responseStream;
		responseStream = NULL;
	}

	responseStream = stream;
	return this;
}

#ifdef ENABLE_SSL
HttpRequest* HttpRequest::setSslOptions(uint32_t sslOptions)
{
	this->sslOptions = sslOptions;
	return this;
}

uint32_t HttpRequest::getSslOptions()
{
	return sslOptions;
}

HttpRequest* HttpRequest::pinCertificate(const SSLFingerprints& fingerprints)
{
	sslFingerprint = fingerprints;
	return this;
}

HttpRequest* HttpRequest::setSslKeyCert(const SSLKeyCertPair& keyCertPair)
{
	this->sslKeyCertPair = keyCertPair;
	return this;
}

#endif

HttpRequest* HttpRequest::setBody(const String& body)
{
	return setBody((uint8_t*)body.c_str(), body.length());
}

HttpRequest* HttpRequest::setBody(uint8_t* rawData, size_t length)
{
	MemoryDataStream* memory = new MemoryDataStream();
	int written = memory->write(rawData, length);
	if (written < length) {
		debug_e("HttpRequest::setBody: Unable to store the complete body");
	}

	return setBody(memory);
}

HttpRequest* HttpRequest::setBody(ReadWriteStream* stream)
{
	if (this->stream != NULL) {
		debug_e("HttpRequest::setBody: Discarding already set stream!");
		delete this->stream;
		this->stream = NULL;
	}

	this->stream = stream;
	return this;
}

HttpRequest* HttpRequest::onBody(RequestBodyDelegate delegateFunction)
{
	requestBodyDelegate = delegateFunction;
	return this;
}

HttpRequest* HttpRequest::onHeadersComplete(RequestHeadersCompletedDelegate delegateFunction)
{
	this->headersCompletedDelegate = delegateFunction;
	return this;
}

HttpRequest* HttpRequest::onRequestComplete(RequestCompletedDelegate delegateFunction)
{
	this->requestCompletedDelegate = delegateFunction;
	return this;
}

void HttpRequest::reset()
{
	delete queryParams;
	delete stream;
	delete responseStream;
	queryParams = NULL;
	stream = NULL;
	responseStream = NULL;

	postParams.clear();
	for (int i = 0; i < files.count(); i++) {
		String key = files.keyAt(i);
		delete files[key];
		files[key] = NULL;
	}
	files.clear();
}

#ifndef SMING_RELEASE
String HttpRequest::toString()
{
	String content = "";
#ifdef ENABLE_SSL
	content += "> SSL options: " + String(sslOptions) + "\n";
	content += "> SSL Cert Fingerprint Length: " + String((sslFingerprint.certSha1 == NULL) ? 0 : SHA1_SIZE) + "\n";
	content += "> SSL PK Fingerprint Length: " + String((sslFingerprint.pkSha256 == NULL) ? 0 : SHA256_SIZE) + "\n";
	content += "> SSL ClientCert Length: " + String(sslKeyCertPair.certificateLength) + "\n";
	content += "> SSL ClientCert PK Length: " + String(sslKeyCertPair.keyLength) + "\n";
	content += "\n";
#endif

	content += http_method_str(method) + String(" ") + uri.getPathWithQuery() + " HTTP/1.1\n";
	content += "Host: " + uri.Host + ":" + uri.Port + "\n";
	for (int i = 0; i < headers.count(); i++) {
		content += headers.keyAt(i) + ": " + headers.valueAt(i) + "\n";
	}

	if (stream != NULL && stream->available() > -1) {
		content += "Content-Length: " + String(stream->available());
	}

	return content;
}
#endif
