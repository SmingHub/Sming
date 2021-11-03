#include <SmingCore.h>
#include <MultipartParser.h>
#include <HttpMultipartResource.h>
#include <Data/Stream/FileStream.h>
#include <Data/Stream/LimitedWriteStream.h>

HttpServer server;

constexpr size_t MAX_FILE_SIZE = 1024; // Allowed size in bytes
String uploadError;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	vars["MAX_FILE_SIZE"] = String(MAX_FILE_SIZE);
	response.sendNamedStream(tmpl); // this template object will be deleted automatically

	response.setCache(86400, true); // It's important to use cache for better performance.
}

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
	auto limitedWriteStream = static_cast<LimitedWriteStream*>(stream);

	String content;
	if(uploadError.length() > 0) {
		content = uploadError;
	}
	else if(!limitedWriteStream->isSuccess()) {
		content = F("File size is bigger than allowed!");
		/*
		 * There is an incomplete file stored on the file system.
		 * You can either leave it as it is and overwrite it the next time or
		 * truncate it with the code below
		 */
		auto fileStream = static_cast<FileStream*>(limitedWriteStream->getSource());
		fileStream->truncate();
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
	 *
	 */


	files["firmware"] = new LimitedWriteStream(MAX_FILE_SIZE, new FileStream());
	/*
	 * The line above defines that firmware should be stored as file stream on the file system
	 * Since no name is provided in the FileStream constructor the name will be injected by the MultipartParser.
	 * Using the LimitedWriteStream wrapper will guarantee that the file size is limited to max  MAX_FILE_SIZE bytes.
	 */
}

void startWebServer()
{
	server.setBodyParser(MIME_FORM_MULTIPART, formMultipartParser);

	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/upgrade", new HttpMultipartResource(fileUploadMapper, onUpload));
	server.paths.setDefault(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system in order to work with files

	WifiStation.enable(true);
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	//WifiStation.enableDHCP(true);

	// Run WEB server on system ready
	System.onReady(startWebServer);
}
