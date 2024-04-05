#include <SmingCore.h>

#define DNS_PORT 53

namespace
{
DnsServer dnsServer;
HttpServer server;

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.setContentType(MIME_HTML);
	response.sendString(F("SMING captive portal"));
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

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	// Start AP
	WifiStation.enable(false);
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config(F("DNSCaptive Portal"), nullptr, AUTH_OPEN);

	System.onReady(startServers);
}
