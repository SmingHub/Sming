#include <user_config.h>
#include <SmingCore/SmingCore.h>

DNSServer dnsServer;
HttpServer server;


const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);


void onDefault(HttpRequest &request, HttpResponse &response)
{
	response.redirect("http://"+apIP.toString());
}

void onIndex(HttpRequest &request, HttpResponse &response) {
	
	response.setContentType(MIME_HTML);
	response.sendString("SMING captive portal");
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onIndex);
}

void startServers() {
	// set DNS server to catch all requests and reply with own ip
	dnsServer.start(DNS_PORT, "*", apIP);
	startWebServer();
}

void init()
{

	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// Start AP 
	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config("DNSCaptive Portal", "", AUTH_OPEN);

	System.onReady(startServers);
}
