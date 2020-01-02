#include <SmingCore.h>
#include <MultipartParser.h>
#include <HttpMultipartResource.h>
#include <OtaUpgradeStream.h>

HttpServer server;

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();
	if(!file.length()) {
		file = "index.html";
	}

	response.setCache(86400, true); // It's important to use cache for better performance.
	response.sendFile(file);
}

int onUpload(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	ReadWriteStream* file = request.files["firmware"];
	OtaUpgradeStream* otaStream = static_cast<OtaUpgradeStream*>(file);
	if(otaStream == nullptr) {
		debug_e("Something went wrong with the file upload");
		return 1;
	}

	if(response.isSuccess() && !otaStream->hasError()) {
		// defer the reboot by 1000 milliseconds to give time to the web server to return the response
		System.restart(1000);

		response.sendFile("restart.html");
		response.headers[HTTP_HEADER_CONNECTION] = "close";

		return 0;
	}

	response.code = HTTP_STATUS_BAD_REQUEST;
	response.setContentType(MIME_HTML);
	String html = "<H2 color='#444'>" + OtaUpgradeStream::errorToString(otaStream->errorCode) + "</H2>";
	response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
	response.sendString(html);

	return 0;
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
	files["firmware"] = new OtaUpgradeStream;
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
