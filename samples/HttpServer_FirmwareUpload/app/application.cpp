#include <SmingCore.h>

#include <Data/Stream/LimitedMemoryStream.h>
#include <MultipartParser/HttpMultipartResource.h>
#include <Data/Stream/RbootOutputStream.h>

HttpServer server;
String lastModified;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	response.sendNamedStream(tmpl);
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	if(lastModified.length() > 0 && request.headers[HTTP_HEADER_IF_MODIFIED_SINCE].equals(lastModified)) {
		response.code = HTTP_STATUS_NOT_MODIFIED;
		return;
	}

	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		if(lastModified.length() > 0) {
			response.headers[HTTP_HEADER_LAST_MODIFIED] = lastModified;
		}

		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

int onUpload(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	ReadWriteStream* file = request.files["firmware"];
	if(file == nullptr) {
		debug_e("Something went wrong with the file upload");
		return 1;
	}

	debugf("Uploaded length: %d", file->available());

	rboot_config bootConfig = rboot_get_config();
	uint8_t slot = bootConfig.current_rom;
	slot = (slot == 0 ? 1 : 0);
	Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
	rboot_set_current_rom(slot);
	System.restart(5); // defer the restart with 5 seconds to give time to the web server to return the response

	response.sendFile("restart.html");

	return 0;
}

void simpleUploadMapper(HttpFiles& files)
{
	/*
	 * On a normal computer the file uploads are usually using
	 * temporary space on the hard disk or in memory to store the incoming data.
	 *
	 * On an embedded device that is a luxury that we can hardly afford.
	 * Therefore we should define a `map` that specifies explicitly
	 * where a specific form field will be stored.
	 *
	 * If a field is not specified then its content will be discarded.
	 */

	// Below we instruct the server to store max 1024 bytes in memory
	// from the incoming data for the "firmware" form fields.
	files["firmware"] = new LimitedMemoryStream(1024);
}

void fileUploadMapper(HttpFiles& files)
{
	/*
	 * On a normal computer the file uploads are usually using
	 * temporary space on the hard disk or in memory to store the incoming data.
	 *
	 * On an embedded device that is a luxury that we can hardly afford.
	 * Therefore we should define a `map` that specifies explicitly
	 * where a specific form field will be stored.
	 *
	 * If a field is not specified then its content will be discarded.
	 */

	// Get the address where the next firmware should be stored.
	rboot_config bootConfig = rboot_get_config();
	uint8_t slot = bootConfig.current_rom;
	slot = (slot == 0 ? 1 : 0);
	int romStartAddress = bootConfig.roms[slot];

	size_t maxLength = 0; // 0  means that there is no max length.
						  // Set this according to your flash memory layout

	files["firmware"] = new RbootOutputStream(romStartAddress, maxLength);
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);

	HttpMultipartResource* uploadResouce = new HttpMultipartResource(fileUploadMapper, onUpload);
	server.paths.set("/upload", uploadResouce);

	server.paths.setDefault(onFile);
}

void init()
{
	spiffs_mount(); // Mount file system, in order to work with files

	if(fileExist(".lastModified")) {
		// The last modification
		lastModified = fileGetContent(".lastModified");
		lastModified.trim();
	}

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	WifiStation.enable(true);

	// Run WEB server on system ready
	System.onReady(startWebServer);
}
