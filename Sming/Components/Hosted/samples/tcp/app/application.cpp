/*
 * This sample application demostrates communication via the TCP
 */

#include <SmingCore.h>
#include <Hosted/Transport/TcpServerStream.h>
#include <simpleRPC.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
using namespace Hosted::Transport;

TcpServer* server = nullptr;
TcpServerStream* transportStream = nullptr;

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	if(server != nullptr) {
		return;
	}

	server = new TcpServer();
	server->listen(4031);
	server->setTimeOut(USHRT_MAX); // disable connection timeout

	transportStream = new TcpServerStream(*server);
	transportStream->onData([](Stream& stream) {
		interface(stream, pinMode, "pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type.",
				  digitalRead, "digitalRead: Read digital pin. @pin: Pin number. @return: Pin value.", digitalWrite,
				  "digitalWrite: Write to a digital pin. @pin: Pin number. @value: Pin value.");

		return true;
	});
}

} // namespace

void init()
{
	// Connect to same AP as the client application
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
}
