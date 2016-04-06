/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef _SMING_CORE_NETWORK_HTTPREQUEST_H_
#define _SMING_CORE_NETWORK_HTTPREQUEST_H_

#define NETWORK_MAX_HTTP_PARSING_LEN 4096

#include "../Wiring/WHashMap.h"
#include "../Wiring/WString.h"

class pbuf;
class HttpServer;
class HttpServerConnection;
class TemplateFileStream;


enum HttpParseResult
{
	eHPR_Wait = 0,
	eHPR_Successful,
	eHPR_Failed,
	eHPR_Failed_Header_Too_Large,
	eHPR_Failed_Body_Too_Large,
	eHPR_Failed_Not_Enough_Space
};

enum MultipartStatus
{
	MULTIPART_START = 0,
	MULTIPART_CONTENT,
	MULTIPART_END,
	MULTIPART_ABORT
};

enum HttpUploadStatus
{
	HTTP_UPLOAD_BEGIN = 0,
	HTTP_UPLOAD_WRITE,
	HTTP_UPLOAD_WRITE_CUSTOM,
	HTTP_UPLOAD_FINISHED,
	HTTP_UPLOAD_SKIP, // don`t save the upload
	HTTP_UPLOAD_ERROR, // internal error
	HTTP_UPLOAD_ABORT // the client aborted / interruped the upload
};

struct HttpUpload {
	String filename;
	String name;
	String type;
	size_t totalSize;
	size_t curSize; // size of the current data block
	char* bufferdata; //pointer to the data
	file_t file;
	HttpUploadStatus status;
};

struct Multipart {
	char* buf;
	size_t buflen;
	String begin_boundary;
	String end_boundary;
	MultipartStatus status;
	HttpUpload* upload;
};

class HttpRequest
{
public:
	HttpRequest();
	virtual ~HttpRequest();

	inline String getRequestMethod() { return method; }
	inline String getPath() { return path; }
	String getContentType();
	int getContentLength();

	bool hasUpload() { return (uploads != NULL); }
	bool isAjax();
	bool isWebSocket();

	String getQueryParameter(String parameterName, String defaultValue = "");
	String getPostParameter(String parameterName, String defaultValue = "");
	String getHeader(String headerName, String defaultValue = "");
	String getCookie(String cookieName, String defaultValue = "");
	String getBody();
	Vector<HttpUpload*> getUploads() { return *uploads; };

public:
	HttpParseResult parseHeader(HttpServer *server, pbuf* buf);
	HttpParseResult parsePostData(HttpServer *server, pbuf* buf);
	HttpParseResult parseMultipartPostData(HttpServer *server, pbuf* buf);
	String extractParsingItemsList(String& buf, int startPos, int endPos,
			char delimChar, char endChar,
			HashMap<String, String>* resultItems);
	String extractHeaderValue(String& buf, String delimChar, String endChar, int startPos = 0);
	void cleanupMultipart(HttpServerConnection& connection);

private:
	String method;
	String path;
	String tmpbuf;
	HashMap<String, String> *requestHeaders;
	HashMap<String, String> *requestGetParameters;
	HashMap<String, String> *requestPostParameters;
	HashMap<String, String> *cookies;
	Vector<HttpUpload*>	*uploads;
	int postDataProcessed;
	int headerDataProcessed;
	int postParamSize;

	Multipart* multipart;

	friend class TemplateFileStream;

private:
	void handleUpload();

};

#endif /* _SMING_CORE_NETWORK_HTTPREQUEST_H_ */
