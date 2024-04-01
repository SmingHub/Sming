#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#define LED_PIN 0 // GPIO number

namespace
{
HttpServer server;
int counter = 0;
HttpClient downloadClient;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	counter++;
	bool led = request.getQueryParameter("led") == "on";
	digitalWrite(LED_PIN, led);

	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	vars["counter"] = String(counter);
	vars["IP"] = WifiStation.getIP().toString();
	vars["MAC"] = WifiStation.getMacAddress().toString();
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
}

void onHello(HttpRequest& request, HttpResponse& response)
{
	response.setContentType(MIME_HTML);

	// Below is an example how to send multiple cookies
	response.setCookie("cookie1", "value1");
	response.setCookie("cookie2", "value", true);

	// Use direct strings output only for small amount of data (huge memory allocation)
	response.sendString("Sming. Let's do smart things.");
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

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.set("/hello", onHello);
	server.paths.setDefault(onFile);

	Serial << endl
		   << _F("=== WEB SERVER STARTED ===") << endl
		   << WifiStation.getIP() << endl
		   << _F("==========================") << endl
		   << endl;
}

void downloadContentFiles()
{
	downloadClient.downloadFile(F("http://simple.anakod.ru/templates/index.html"));
	downloadClient.downloadFile(F("http://simple.anakod.ru/templates/bootstrap.css.gz"));
	downloadClient.downloadFile(F("http://simple.anakod.ru/templates/jquery.js.gz"),
								[](HttpConnection& connection, bool success) -> int {
									if(success) {
										startWebServer();
									}
									return 0;
								});
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	if(!fileExist("index.html") || !fileExist("bootstrap.css.gz") || !fileExist("jquery.js.gz")) {
		// Download server content at first
		downloadContentFiles();
	} else {
		startWebServer();
	}
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount(); // Mount file system, in order to work with files

	pinMode(LED_PIN, OUTPUT);

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false);

	// Run our method when station was connected to AP
	WifiEvents.onStationGotIP(gotIP);

	// Max. out CPU frequency
	System.setCpuFrequency(CpuCycleClockFast::cpuFrequency());
	Serial << _F("New CPU frequency is ") << System.getCpuFrequency() << " MHz" << endl;
}
