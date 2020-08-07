#include <SmingCore.h>
#include <HostedServer.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

HostedServer hostedServer;
TcpServer* tcpServer;

namespace Hosted {
	namespace Digital {
		void registerCommands(HostedServer& server);
	}
	namespace Spi {
		void registerCommands(HostedServer& server);
	}
}

// Will be called when WiFi station was connected to AP
void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	// Start the TcpServer
	if(tcpServer != nullptr) {
		return;
	}

	tcpServer = new TcpServer([](TcpClient& client, char* data, int size) -> bool {
		// clientReceiveDataHandler
		int result = hostedServer.process((const uint8_t*)data, size);
		if(result != HOSTED_OK) {
			return result == HOSTED_NO_MEM ? false: true;
		}

		hostedServer.transfer([&client](const uint8_t* data, size_t size) -> bool {
			return client.send((const char*)data, size);
		});

		return true;
	});

	tcpServer->listen(4031);
	tcpServer->setTimeOut(USHRT_MAX); // disable connection timeout
}

void init()
{
	Hosted::Digital::registerCommands(hostedServer);
	Hosted::Spi::registerCommands(hostedServer);


	// Connect to same AP as the client application
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);
}

