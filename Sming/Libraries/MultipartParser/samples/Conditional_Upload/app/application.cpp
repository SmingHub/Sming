#include <SmingCore.h>
#include <MultipartParser.h>
#include <HttpMultipartResource.h>
#include <PartCheckerStream.h>
#include <Data/Stream/FileStream.h>
#include <Data/Stream/LimitedWriteStream.h>
#include <PartCheckerStream.h>
#include <FlashString/Array.hpp>

HttpServer server;

constexpr size_t MAX_FILE_SIZE = 1024; // Allowed size in bytes
String uploadError;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	vars["MAX_FILE_SIZE"] = String(MAX_FILE_SIZE);
	response.sendNamedStream(tmpl); // this template object will be deleted automatically

	response.setCache(86400, true); // You can instruct the browser to cache the content for better performance.
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
	response.code = HTTP_STATUS_OK;
	response.setContentType(MIME_HTML);
	String content = F("All good");

	ReadWriteStream* stream = request.files["firmware"];

	if(stream != nullptr && stream->getStreamType() == eSST_HeaderChecker) {
		auto checkerStream = static_cast<PartCheckerStream*>(stream);
		if(!checkerStream->isValid()) {
			response.code = HTTP_STATUS_BAD_REQUEST;
			content = uploadError;
		}
	}
	else {
		auto limitedWriteStream = static_cast<LimitedWriteStream*>(stream);


		if(uploadError.length() > 0) {
			response.code = HTTP_STATUS_BAD_REQUEST;
			content = uploadError;
		}
		else if(!limitedWriteStream->isValid()) {
			content = F("File size is bigger than allowed!");
			response.code = HTTP_STATUS_BAD_REQUEST;
			/*
			 * There is an incomplete file stored on the file system.
			 * You can either leave it as it is and overwrite it the next time or
			 * truncate it with the code below
			 */
			auto fileStream = static_cast<FileStream*>(limitedWriteStream->getSource());
			fileStream->truncate();
		}
	}

	String html = F("<H2 color='#444'>") + content + F("</H2>");
	response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
	response.sendString(html);

	return 0;
}

/**
 * @brief This function is used to check if an upload file can be stored.
 *
 * @param headers - HTTP headers for the specific part
 * @param stream - the original stream used to store data
 * @param part
 *
 * @retval bool false to reject the saving of the content
 */
bool allowPart(const HttpHeaders& headers, ReadWriteStream* stream, const PartCheckerStream::FilePart& part)
{
	// below is an example how to check for the filename length before storing it
	if(part.fileName.length() > 32) {
		uploadError = F("Filename too long!");
		return false;
	}

	// If needed it is possible also to check if the file type is as expected
	// The code below requires the uploaded file to be plain text.
	if(part.mime != toString(MIME_TEXT)) {
		uploadError = F("Only text files allowed!");
		return false;
	}

	// here is an example how to set the provided file name
	if(part.fileName.length() > 0) {
		auto fileStream = static_cast<FileStream*>(stream);
		fileStream->open(part.fileName, File::CreateNewAlways | File::WriteOnly);
	}

	return true;
}

void fileUploadMapper(HttpFiles& files)
{
	uploadError = nullptr; // Reset to 'no error'

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

	/*
	 * Create a stream to store uploaded content.
	 * Since no name is provided in the constructor the name will be injected by the MultipartParser.
	 */
	auto fileStream = new FileStream;

	/*
	 * Using the LimitedWriteStream wrapper will guarantee that the file size is limited to MAX_FILE_SIZE bytes.
	 */
	// files["firmware"] = new LimitedWriteStream(fileStream, MAX_FILE_SIZE);

	/**
	 * Uncomment the line below for a more complex example.
	 * This one uses PartCheckerStream and provides way to decide if the content should be stored
	 * based on the allowPart callback.
	 */
	files["firmware"] = new PartCheckerStream(fileStream, allowPart);
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
