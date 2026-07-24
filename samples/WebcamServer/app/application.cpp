#include <SmingCore.h>

#include <WebcamStream.h>
#include <Camera/FakeCamera.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
HttpServer server;
FakeCamera camera;

/*
 * default http handler to check if server is up and running
 */

void onIndex(HttpRequest&, HttpResponse& response)
{
	response.sendFile(_F("index.html"));
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void onStream(HttpRequest& request, HttpResponse& response)
{
	Serial.println(_F("perform onCapture()"));

	WebcamStream* stream = new WebcamStream(camera);
	response.sendDataStream(stream, F("multipart/x-mixed-replace; boundary=") + stream->getBoundary());
}

void onFavicon(HttpRequest& request, HttpResponse& response)
{
	response.code = HTTP_STATUS_NOT_FOUND;
}

// Will be called when station is fully operational
void startWebServer()
{
	// Initialize the camera
	for(unsigned int i = 1; i < 6; i++) {
		String s = "img";
		s.concat(i, DEC, 2);
		s += ".jpeg";
		camera.addImage(s);
	}

	spiffs_mount(); // Mount file system, in order to work with files
	camera.init();

	// .. and run the HTTP server
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/stream", onStream);
	server.paths.set("/favicon.ico", onFavicon);
	server.paths.setDefault(onFile);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // Enable serial
	Serial.systemDebugOutput(true); // Allow debug output to serial

	spiffs_mount();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	System.onReady(startWebServer);
}
