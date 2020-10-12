#include <SmingCore.h>
#include <Network/Mdns/Responder.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

DEFINE_FSTR_LOCAL(hostName, "UDP_Server");

HttpServer server;

class MyService : public mDNS::Service
{
public:
	Info getInfo() override
	{
		Info info;
		info.name = F("Sming");
		return info;
	}

	CStringArray getTxt() override
	{
		return F("version=now");
	}
};

static mDNS::Responder responder;
static MyService service;

void startmDNS()
{
	responder.begin(hostName);
	responder.addService(service);
}

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.sendFile("index.html");
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
	if(!fileExist("index.html")) {
		String content = F("<h3>Congrats !! You are Connected to your ESP module with mDNS address %host%.local</h3>");
		content.replace(F("%host%"), hostName);
		fileSetContent("index.html", content);
	}
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
