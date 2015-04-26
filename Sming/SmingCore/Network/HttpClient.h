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

class HttpClient;
class URL;

typedef void (*HttpClientCompletedCallback)(HttpClient& client, bool successful);

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
	bool downloadString(String url, HttpClientCompletedCallback onCompleted);
	String getResponseString(); // Can be used only after calling downloadString!

	// File mode
	bool downloadFile(String url, HttpClientCompletedCallback onCompleted = NULL);
	bool downloadFile(String url, String saveFileName, HttpClientCompletedCallback onCompleted = NULL);

	// Resulting HTTP status code
	inline int getReponseCode() { return code; }

	inline bool isProcessing()  { return TcpClient::isProcessing(); }
	inline bool isSuccessful() { return (!writeError) && (code >= 200 && code <= 399); }

	String getResponseHeader(String headerName, String defaultValue = "");
	DateTime getLastModifiedDate(); // Last-Modified header
	DateTime getServerDate(); // Date header

	void reset(); // Reset current status, data and etc.

protected:
	bool startDownload(URL uri, HttpClientMode mode, HttpClientCompletedCallback onCompleted);
	void onFinished(TcpClientState finishState);
	virtual err_t onReceive(pbuf *buf);
	virtual void writeRawData(pbuf* buf, int startPos);
	void parseHeaders(pbuf* buf, int headerEnd);

protected:
	bool waitParse;
	bool writeError;

private:
	int code;
	HttpClientCompletedCallback onCompleted;
	HttpClientMode mode;
	HashMap<String, String> responseHeaders;

	String responseStringData;
	file_t saveFile;
};

#endif /* _SMING_CORE_NETWORK_HTTPCLIENT_H_ */
