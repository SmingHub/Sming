#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HttpServer server;

//mDNS using ESP8266 SDK functions
void startmDNS()
{
	struct mdns_info* info = (struct mdns_info*)os_zalloc(sizeof(struct mdns_info));
	info->host_name = (char*)"test"; // You can replace test with your own host name
	info->ipAddr = WifiStation.getIP();
	info->server_name = (char*)"Sming";
	info->server_port = 80;
	info->txt_data[0] = (char*)"version = now";
	espconn_mdns_init(info);
}

void onIndex(HttpRequest& request, HttpResponse& response)
{
	TemplateFileStream* tmpl = new TemplateFileStream("index.html");
	auto& vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendNamedStream(tmpl); // this template object will be deleted automatically
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

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	if(!fileExist("index.html"))
		fileSetContent("index.html",
					   "<h3>Congrats !! You are Connected to your ESP module with mDNS address test.local</h3>");
	startWebServer();
	startmDNS(); // Start mDNS "Advertise" of your hostname "test.local" for this example
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount();

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
