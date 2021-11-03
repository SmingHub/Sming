#include <SmingCore.h>
#include <MultipartParser.h>
#include <HttpMultipartResource.h>
#include <PartCheckerStream.h>
#include <Data/Stream/FileStream.h>
#include <Data/Stream/LimitedWriteStream.h>
#include <FlashString/Array.hpp>

HttpServer server;

constexpr size_t MAX_FILE_SIZE = 1024; // Allowed size in bytes
String uploadError;

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();
	if(file.length() == 0) {
		file = "index.html";
	}

	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile(file);
}

int onUpload(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	response.code = HTTP_STATUS_BAD_REQUEST;
	response.setContentType(MIME_HTML);

	ReadWriteStream* stream = request.files["firmware"];
	auto partCheckerStream = static_cast<PartCheckerStream*>(stream);
	auto limitedWriteStream = static_cast<LimitedWriteStream*>(partCheckerStream->getSource());

	String content;
	if(uploadError.length() > 0) {
		content = uploadError;
	}
	else if(!limitedWriteStream->isSuccess()) {
		content = F("File size is bigger than allowed!");
	}
	else {
		response.code = HTTP_STATUS_OK;
		content = F("All good");
	}

	String html = "<H2 color='#444'>" + content + "</H2>";
	response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
	response.sendString(html);

	return 0;
}

bool allowUpload(const HttpHeaders& headers, ReadWriteStream* stream, const String& fileName)
{
	if(fileName.length() > 0) {
		auto limitedStream = static_cast<LimitedWriteStream*>(stream);
		auto fileStream = static_cast<FileStream*>(limitedStream->getSource());
		if(!fileStream->open(fileName, File::CreateNewAlways | File::WriteOnly)) {
			uploadError = "Unable to save the remote file locally";
			return false;
		}
	}

	return true;
}

void fileUploadMapper(HttpFiles& files)
{
	/*
	 * On a normal computer file uploads are usually using
	 * temporary space on the hard disk or in memory to store the incoming data.
	 *
	 * On an embedded device that is a luxury that we can hardly afford.
	 * Therefore we should define a `map` that specifies explicitly
	 * by which stream each form field will be consumed.
	 *
	 * If a field is not specified then its content will be discarded.
	 */

	auto limitedFileStream = new LimitedWriteStream(MAX_FILE_SIZE, new FileStream());

	files["firmware"] = new PartCheckerStream(allowUpload, limitedFileStream);
}

void startWebServer()
{
	HttpServerSettings settings;
	/* 
	 * If an error is detected early in a request's message body (like an attempt to upload a firmware image for the 
	 * wrong slot), the default behaviour of Sming's HTTP server is to send the error response as soon as possible and 
	 * then close the connection.
	 * However, some HTTP clients (most notably Firefox!) start listening for a response only after having transmitted 
	 * the whole request. Such clients may miss the error response entirely and instead report to the user that the 
	 * connection was closed unexpectedly. Disabling 'closeOnContentError' instructs the server to delay the error 
	 * response until after the whole message body has been received. This allows all clients to receive the response 
	 * and display the exact error message to the user, leading to an overall improved user experience.
	 */
	settings.closeOnContentError = false;
	settings.keepAliveSeconds = 2; // default from HttpServer::HttpServer()
	server.configure(settings);
	server.setBodyParser(MIME_FORM_MULTIPART, formMultipartParser);

	server.listen(80);
	server.paths.set("/upgrade", new HttpMultipartResource(fileUploadMapper, onUpload));
	server.paths.setDefault(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	WifiStation.enable(true);
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	//WifiStation.enableDHCP(true);

	// Run WEB server on system ready
	System.onReady(startWebServer);
}
