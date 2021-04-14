/*
 * This sample application demostrates RPC communication via TCP.
 * It will try to connect to create an existing Access Point (AP) or create to a new AP and start a TCP server.
 * The TCP server will listen on port 4031 for remote commands.
 */
#include <SmingCore.h>
#include <simpleRPC.h>
#include <Hosted/Transport/TcpServerTransport.h>

#if CONNECT_TO_WIFI
// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif
#endif /* CONNECT_TO_WIFI */

namespace
{
constexpr size_t port = 4031;

using namespace Hosted::Transport;

TcpServer* server = nullptr;
TcpServerTransport* transport = nullptr;

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	if(server != nullptr) {
		return;
	}

	server = new TcpServer();
	server->listen(port);
	server->setTimeOut(USHRT_MAX); // disable connection timeout

	transport = new TcpServerTransport(*server);
	transport->onData([](Stream& stream) {
		// clang-format off
		interface(stream,
			/*
			 * Below we are exporting the following remote commands:
			 * - pinMode
			 * - digitalRead
			 * - digitalWrite
			 * You can add more commands here. For every command you should specify command and text description in the format below.
			 * For more information read the SimpleRPC interface API: https://simplerpc.readthedocs.io/en/latest/api/interface.html
			 */
			pinMode, F("pinMode: Sets mode of digital pin. @pin: Pin number, @mode: Mode type."),
			digitalRead, F("digitalRead: Read digital pin. @pin: Pin number. @return: Pin value."),
			digitalWrite, F("digitalWrite: Write to a digital pin. @pin: Pin number. @value: Pin value.")
		);
		// clang-format on

		return true;
	});

	Serial.printf("Running RCP server on: %s:%u", ip.toString().c_str(), port);
}

} // namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);

#if CONNECT_TO_WIFI
	// Connect to same AP as the client application
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
#else
	WifiAccessPoint.enable(true);
	WifiAccessPoint.config(_F("RCP Server"), nullptr, AUTH_OPEN);
	connectOk(WifiAccessPoint.getIP(), WifiAccessPoint.getNetworkMask(), WifiAccessPoint.getNetworkGateway());
#endif
}
