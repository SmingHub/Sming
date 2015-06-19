/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTPCLIENT_H_
#define _SMING_CORE_NETWORK_HTTPCLIENT_H_

#include "TcpClient.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WHashMap.h"
#include "../../Services/DateTime/DateTime.h"
#include "../Delegate.h"

class HttpClient;
class URL;

//typedef void (*HttpClientCompletedCallback)(HttpClient& client, bool successful);
typedef Delegate<void(HttpClient& client, bool successful)> HttpClientCompletedDelegate;

enum HttpClientMode
{
	eHCM_String = 0,
	eHCM_File,
	eHCM_UserDefined
};

class HttpClient: protected TcpClient
{
public:
	HttpClient(bool autoDestruct = false);
	virtual ~HttpClient();

	// Text mode
	bool downloadString(String url, HttpClientCompletedDelegate onCompleted);
	String getResponseString(); // Can be used only after calling downloadString!

	// File mode
	bool downloadFile(String url, HttpClientCompletedDelegate onCompleted = NULL);
	bool downloadFile(String url, String saveFileName, HttpClientCompletedDelegate onCompleted = NULL);

	void setPostBody(const String& _method);
	String getPostBody();

	void setRequestHeader(const String name, const String value);
	bool hasRequestHeader(const String name);
	void setRequestContentType(String _content_type);

	// Resulting HTTP status code
	__forceinline int getReponseCode() { return code; }
	__forceinline bool isSuccessful() { return (!writeError) && (code >= 200 && code <= 399); }

	__forceinline bool isProcessing()  { return TcpClient::isProcessing(); }
	__forceinline TcpClientState getConnectionState() { return TcpClient::getConnectionState(); }

	String getResponseHeader(String headerName, String defaultValue = "");
	DateTime getLastModifiedDate(); // Last-Modified header
	DateTime getServerDate(); // Date header

	void reset(); // Reset current status, data and etc.

protected:
	bool startDownload(URL uri, HttpClientMode mode, HttpClientCompletedDelegate onCompleted);
	void onFinished(TcpClientState finishState);
	virtual err_t onReceive(pbuf *buf);
	virtual void writeRawData(pbuf* buf, int startPos);
	void parseHeaders(pbuf* buf, int headerEnd);

protected:
	bool waitParse = false;
	bool writeError = false;

private:
	int code;
	HttpClientCompletedDelegate onCompleted;
	HttpClientMode mode;
	HashMap<String, String> requestHeaders;
	HashMap<String, String> responseHeaders;

	String responseStringData;
	String body = "";
	file_t saveFile;
};

#endif /* _SMING_CORE_NETWORK_HTTPCLIENT_H_ */
