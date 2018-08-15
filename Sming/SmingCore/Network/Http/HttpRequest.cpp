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
#include "Data/Stream/MemoryDataStream.h"
#include "Data/Stream/ChunkedStream.h"
#include "Data/Stream/UrlencodedOutputStream.h"

HttpRequest::HttpRequest(const HttpRequest& value) : uri(value.uri)
{
	*this = value;
	method = value.method;
	headers = value.headers;

	_headersCompletedDelegate = value._headersCompletedDelegate;
	_requestBodyDelegate = value._requestBodyDelegate;
	_requestCompletedDelegate = value._requestCompletedDelegate;

	debug_w("Warning: HttpRequest streams are not copied..");

#ifdef ENABLE_SSL
	_sslOptions = value._sslOptions;
	_sslFingerprint = value._sslFingerprint;
	_sslKeyCertPair = value._sslKeyCertPair;
#endif
}

HttpRequest& HttpRequest::operator=(const HttpRequest& rhs)
{
	if (this == &rhs)
		return *this;

/*
	@todo: FIX this... How ? method requires a precise explanation first

		if (rhs.buffer) copy(rhs.buffer, rhs.len);
		else invalidate();
*/

	return *this;
}

String HttpRequest::getBody()
{
	if (!_bodyStream || _bodyStream->getStreamType() != eSST_Memory)
		return nullptr;

	int len = _bodyStream->available();
	if (len <= 0)
		return nullptr;

	String s;
	if (s.setLength(len))
		_bodyStream->readMemoryBlock(s.begin(), s.length());

	return s;
}

String HttpRequest::getRequestLine()
{
	return methodStr() + ' ' + uri.pathWithQuery() + _F(" HTTP/1.1\r\n");
}

HttpHeaders& HttpRequest::prepareHeaders()
{
	if (!headers.contains(hhfn_Host))
		headers[hhfn_Host] = uri.host();

	headers[hhfn_ContentLength] = "0";
	if (_files.count()) {
		auto mStream = new MultipartStream(HttpPartProducerDelegate(&HttpRequest::multipartProducer, this));
		headers[hhfn_ContentType] =
			ContentType::toString(MIME_FORM_MULTIPART) + F("; boundary=") + mStream->getBoundary();
		setBody(mStream);
	}
	else if (postParams.count()) {
		auto uStream = new UrlencodedOutputStream(postParams);
		headers[hhfn_ContentType] = ContentType::toString(MIME_FORM_URL_ENCODED);
		setBody(uStream);
	}

	if (_bodyStream) {
		int length = _bodyStream->available();
		if (length >= 0)
			headers[hhfn_ContentLength] = String(length);
		else
			headers.remove(hhfn_ContentLength);
	}

	if (!headers.contains(hhfn_ContentLength))
		headers[hhfn_TransferEncoding] = F("chunked");

	return headers;
}

HttpPartResult HttpRequest::multipartProducer()
{
	HttpPartResult result;

	String name;
	auto file = _files.extract(0, name);
	if (file) {
		result.stream = file;

		HttpHeaders* headers = new HttpHeaders();
		String filename = file->name();
		(*headers)[hhfn_ContentDisposition] = F("form-data; name=\"") + name + F("\"; filename=\"") + filename + '"';
		(*headers)[hhfn_ContentType] = ContentType::fromFileName(filename);

		return result;
	}

	String value = postParams.extract(0, name);
	if (value) {
		MemoryDataStream* mStream = new MemoryDataStream();
		mStream->print(value);
		result.stream = mStream;

		HttpHeaders* headers = new HttpHeaders();
		(*headers)[hhfn_ContentDisposition] = F("form-data; name=\"") + name + "\"";
		result.headers = headers;

		return result;
	}

	return result;
}

IDataSourceStream* HttpRequest::getBodyStream()
{
	auto s = _bodyStream;
	_bodyStream = nullptr;

	if (s && headers[hhfn_TransferEncoding] == F("chunked"))
		return new ChunkedStream(s);
	else
		return s;
}

HttpRequest* HttpRequest::setResponseStream(ReadWriteStream* stream)
{
	if (_responseStream) {
		debug_e("HttpRequest::setResponseStream: Discarding already set stream!");
		delete _responseStream;
	}

	_responseStream = stream;
	return this;
}

HttpRequest* HttpRequest::setBody(uint8_t* rawData, size_t length)
{
	auto memory = new MemoryDataStream();
	auto written = memory->write(rawData, length);
	if (written < length)
		debug_e("HttpRequest::setBody: Unable to store the complete body");

	return setBody(memory);
}

HttpRequest* HttpRequest::setBody(IDataSourceStream* stream)
{
	if (_bodyStream) {
		debug_e("HttpRequest::setBody: Discarding already set stream!");
		delete _bodyStream;
	}

	_bodyStream = stream;
	return this;
}

void HttpRequest::reset()
{
	delete _bodyStream;
	_bodyStream = nullptr;

	delete _responseStream;
	_responseStream = nullptr;

#ifdef ENABLE_SSL
	freeSSLFingerprints(_sslFingerprint);
	freeSslKeyCert(_sslKeyCertPair);
#endif

	postParams.clear();
	_files.clear();
}

#ifndef SMING_RELEASE
String HttpRequest::toString()
{
	String content;
#ifdef ENABLE_SSL
	content += "> SSL options: " + String(_sslOptions) + "\n";
	content += "> SSL Cert Fingerprint Length: " + String(_sslFingerprint.certSha1 ? SHA1_SIZE : 0) + "\n";
	content += "> SSL PK Fingerprint Length: " + String(_sslFingerprint.pkSha256 ? SHA256_SIZE : 0) + "\n";
	content += "> SSL ClientCert Length: " + String(_sslKeyCertPair.certificateLength) + "\n";
	content += "> SSL ClientCert PK Length: " + String(_sslKeyCertPair.keyLength) + "\n";
	content += "\n";
#endif

	content += getRequestLine();
	content += headers.toString(hhfn_Host, uri.hostWithPort());
	content += "Host: " + uri.host() + ':' + uri.port() + '\n';
	for (unsigned i = 0; i < headers.count(); i++)
		content += headers[i];

	if (_bodyStream && _bodyStream->available() >= 0)
		content += HttpHeaders::toString(hhfn_ContentLength) + ": " + String(_bodyStream->available());

	return content;
}
#endif
