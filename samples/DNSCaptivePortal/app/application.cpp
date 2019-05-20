#include <SmingCore.h>

DNSServer dnsServer;
HttpServer server;

#define DNS_PORT 53

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.setContentType(MIME_HTML);
	response.sendString("SMING captive portal");
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);
	server.paths.setDefault(onIndex);
}

void startServers()
{
	// set DNS server to catch all requests and reply with own ip
	dnsServer.start(DNS_PORT, "*", WifiAccessPoint.getIP());
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
