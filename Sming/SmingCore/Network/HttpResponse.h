/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTPRESPONSE_H_
#define _SMING_CORE_NETWORK_HTTPRESPONSE_H_

#include "../../SmingCore/DataSourceStream.h"
#include "WebConstants.h"
#include "../Wiring/WHashMap.h"
#include "../Wiring/WString.h"

class pbuf;
class HttpServer;
class HttpServerConnection;

class HttpResponse
{
public:
	HttpResponse();
	virtual ~HttpResponse();

	void switchingProtocols();
	void badRequest();
	void notFound();
	void forbidden();
	void authorizationRequired();
	void redirect(String location = "");

	void setContentType(const String type);
	void setCookie(const String name, const String value);
	void setHeader(const String name, const String value);
	bool hasHeader(const String name);

	void setCache(int maxAgeSeconds = 3600, bool isPublic = false);
	void setAllowCrossDomainOrigin(String controlAllowOrigin); // Access-Control-Allow-Origin for AJAX from a different domain

	String getStatusName();
	int getStatusCode();
	bool hasBody();

	//*** This methods processed in background

	// Send string. Large memory allocation, only for short strings!
	void sendString(const char* string);
	void sendString(String string);

	// Send file by name
	bool sendFile(String fileName, bool allowGzipFileCheck = true);

	// Parse and send template file
	bool sendTemplate(TemplateFileStream* newTemplateInstance);

	// Build and send JSON string
	bool sendJsonObject(JsonObjectStream* newJsonStreamInstance);
	//***

public:
	void sendHeader(HttpServerConnection &connection);
	bool sendBody(HttpServerConnection &connection);

private:
	bool headerSent;
	bool bodySent;
	String status;
	HashMap<String, String> responseHeaders;

	IDataSourceStream* stream;
};

#endif /* _SMING_CORE_NETWORK_HTTPRESPONSE_H_ */
