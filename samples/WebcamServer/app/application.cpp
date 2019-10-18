#include <SmingCore.h>

#include <Data/Stream/MultipartStream.h>
#include <WebcamStream.h>
#include <Camera/FakeCamera.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;
FakeCamera* camera = nullptr;

/*
 * default http handler to check if server is up and running
 */

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.sendFile(_F("index.html"));
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.')
		response.code = HTTP_STATUS_FORBIDDEN;
	else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

HttpPartResult snapshotProducer()
{
	HttpPartResult result;

	WebcamStream* webcamStream = new WebcamStream(camera);
	result.stream = webcamStream;

	result.headers = new HttpHeaders();
	(*result.headers)["Content-Type"] = camera->getMimeType();

	return result;
}

void onStream(HttpRequest& request, HttpResponse& response)
{
	Serial.printf("perform onCapture()\r\n");

	MultipartStream* stream = new MultipartStream(snapshotProducer);
	response.sendDataStream(stream, String("multipart/x-mixed-replace; boundary=") + stream->getBoundary());
}

void onFavicon(HttpRequest& request, HttpResponse& response)
{
	response.code = HTTP_STATUS_NOT_FOUND;
}

// Will be called when station is fully operational
void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	// Initialize the camera
	Vector<String> images;
	char buf[13] = {0};
	for(unsigned int i = 1; i < 10; i++) {
		sprintf(buf, "img00%02d.jpeg", i);
		images.add(buf);
	}
	camera = new FakeCamera(images);
	camera->init();

	// .. and run the HTTP server
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/stream", onStream);
	server.paths.set("/favicon.ico", onFavicon);
	server.paths.setDefault(onFile);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Allow debug output to serial

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationGotIP(gotIP);
}
