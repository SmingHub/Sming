#include <SmingCore.h>
#include <HostedServer.h>
#include <Digital.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HostedServer hostedServer;
TcpServer* tcpServer;

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	// Start the TcpServer
	tcpServer = new TcpServer([](TcpClient& client, char* data, int size) -> bool {
		hostedServer.process((const uint8_t*)data, size);
		// TODO: check if the processed bytes are equal to the provided bytes
		return true;
	});

	tcpServer->listen(4031);
	tcpServer->setTimeOut(USHRT_MAX); // disable connection timeout
}

void init()
{
	// Register Command Handlers
	hostedServer.registerCommand(HostedMessageType_TypeRequestPinMode, [](HostedCommand *request, HostedCommand *response)-> int {
		pinMode((uint16_t)request->payload.requestPinMode.pin, (uint8_t)request->payload.requestPinMode.mode);
		return 0;
	});

	hostedServer.registerCommand(HostedMessageType_TypeRequestDigitalWrite, [](HostedCommand *request, HostedCommand *response)-> int {
		digitalWrite((uint16_t)request->payload.requestDigitalWrite.pin, (uint8_t)request->payload.requestDigitalWrite.value);
		return 0;
	});

	// Connect to same AP as the client application
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
}

