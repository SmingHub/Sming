#include <SmingCore.h>

#include <Data/Stream/LimitedMemoryStream.h>
#include <MultipartParser/HttpMultipartResource.h>
#include "SignedRbootOutputStream.h"
#include "FirmwareVerificationKey.h"

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
	SignedRbootOutputStream* uploadStream = static_cast<SignedRbootOutputStream*>(file);
	if(uploadStream == nullptr) {
		debug_e("Something went wrong with the file upload");
		return 1;
	}

	if(response.isSuccess() && uploadStream->verifySignature(firmwareVerificationKey)) {
		rboot_config bootConfig = rboot_get_config();
		uint8_t slot = bootConfig.current_rom;
		slot = (slot == 0 ? 1 : 0);
		Serial.printf("Firmware updated, rebooting to rom %d...\r\n", slot);
		rboot_set_current_rom(slot);
		// defer the reboot with 1000 milliseconds to give time to the web server to return the response
		System.restart(1000);

		response.sendFile("restart.html");
		response.headers[HTTP_HEADER_CONNECTION] = "close";

		return 0;
	}

	response.code = HTTP_STATUS_BAD_REQUEST;
	response.setContentType(MIME_HTML);
	String html = "<H2 color='#444'>" + uploadStream->errorMessage + "</H2>";
	response.headers[HTTP_HEADER_CONTENT_LENGTH] = html.length();
	response.sendString(html);

	return 0;
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
	const rboot_config bootConfig = rboot_get_config();
	uint8_t currentSlot = bootConfig.current_rom;
	uint8_t slot = (currentSlot == 0 ? 1 : 0);
	auto romStartAddress = bootConfig.roms[slot];

	size_t maxLength = 0x100000 - (romStartAddress & 0xFFFFF);
	if(bootConfig.roms[currentSlot] > romStartAddress) {
		maxLength = std::min(maxLength, bootConfig.roms[currentSlot] - romStartAddress);
	}
	if(RBOOT_SPIFFS_0 > romStartAddress) {
		maxLength = std::min<size_t>(maxLength, RBOOT_SPIFFS_0);
	}

	files["firmware"] = new SignedRbootOutputStream(romStartAddress, maxLength);
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

	server.listen(80);
	server.paths.set("/", onIndex);

	HttpMultipartResource* uploadResouce = new HttpMultipartResource(fileUploadMapper, onUpload);
	server.paths.set("/upload", uploadResouce);

	server.paths.setDefault(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	if(fileExist(".lastModified")) {
		// The last modification
		lastModified = fileGetContent(".lastModified");
		lastModified.trim();
	}

	WifiStation.enable(true);
	//WifiStation.config(WIFI_SSID, WIFI_PWD);
	//WifiStation.enableDHCP(true);

	// Run WEB server on system ready
	System.onReady(startWebServer);
}
