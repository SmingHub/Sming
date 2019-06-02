/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * HttpRequest.cpp
 *
 * @author: 2017 - Slavey Karadzhov <slav@attachix.com>
 *
 ****/

#include "HttpRequest.h"
#include "Data/Stream/MemoryDataStream.h"

HttpRequest::HttpRequest(const HttpRequest& value)
	: uri(value.uri), method(value.method), headers(value.headers), postParams(value.postParams),
	  headersCompletedDelegate(value.headersCompletedDelegate), requestBodyDelegate(value.requestBodyDelegate),
	  requestCompletedDelegate(value.requestCompletedDelegate)
#ifdef ENABLE_SSL
	  ,
	  sslOptions(value.sslOptions), sslFingerprints(value.sslFingerprints), sslKeyCertPair(value.sslKeyCertPair)
#endif
{
}

String HttpRequest::getBody()
{
	if(bodyStream == nullptr || bodyStream->getStreamType() != eSST_Memory) {
		return nullptr;
	}

	int len = bodyStream->available();
	if(len <= 0) {
		// Cannot determine body size so need to use stream
		return nullptr;
	}

	String ret;
	if(ret.setLength(len)) {
		len = bodyStream->readMemoryBlock(ret.begin(), len);
		// Just in case read count is less than reported count
		ret.setLength(len);
	}

	return ret;
}

IDataSourceStream* HttpRequest::getBodyStream()
{
	return bodyStream;
}

HttpRequest* HttpRequest::setResponseStream(ReadWriteStream* stream)
{
	if(responseStream != nullptr) {
		debug_e("HttpRequest::setResponseStream: Discarding already set stream!");
		delete responseStream;
	}

	responseStream = stream;
	return this;
}

HttpRequest* HttpRequest::setBody(const uint8_t* rawData, size_t length)
{
	auto memory = new MemoryDataStream();
	auto written = memory->write(rawData, length);
	if(written < length) {
		debug_e("HttpRequest::setBody: Unable to store the complete body");
	}

	return setBody(memory);
}

HttpRequest* HttpRequest::setBody(IDataSourceStream* stream)
{
	if(bodyStream != nullptr) {
		debug_e("HttpRequest::setBody: Discarding already set stream!");
		delete bodyStream;
	}

	bodyStream = stream;
	return this;
}

void HttpRequest::reset()
{
	delete queryParams;
	queryParams = nullptr;

	delete bodyStream;
	bodyStream = nullptr;

	delete responseStream;
	responseStream = nullptr;

	postParams.clear();
	files.clear();
}

#ifndef SMING_RELEASE
String HttpRequest::toString()
{
	String content;
#ifdef ENABLE_SSL
	content += F("> SSL options: ") + String(sslOptions) + '\n';
	content +=
		F("> SSL Cert Fingerprint Length: ") + String((sslFingerprints.certSha1 == nullptr) ? 0 : SHA1_SIZE) + '\n';
	content +=
		F("> SSL PK Fingerprint Length: ") + String((sslFingerprints.pkSha256 == nullptr) ? 0 : SHA256_SIZE) + '\n';
	content += F("> SSL ClientCert Length: ") + String(sslKeyCertPair.getCertificateLength()) + '\n';
	content += F("> SSL ClientCert PK Length: ") + String(sslKeyCertPair.getKeyLength()) + '\n';
	content += '\n';
#endif

	content += String(http_method_str(method)) + ' ' + uri.getPathWithQuery() + _F(" HTTP/1.1\n");
	content += headers.toString(HTTP_HEADER_HOST, uri.getHostWithPort());
	for(unsigned i = 0; i < headers.count(); i++) {
		content += headers[i];
	}

	if(bodyStream != nullptr && bodyStream->available() >= 0) {
		content += headers.toString(HTTP_HEADER_CONTENT_LENGTH, String(bodyStream->available()));
	}

	return content;
}
#endif
