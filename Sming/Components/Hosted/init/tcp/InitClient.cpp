/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * InitClient.cpp
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#include <Platform/WifiEvents.h>
#include <Platform/Station.h>
#include <Hosted/Client.h>
#include <Hosted/Transport/TcpClientStream.h>

Hosted::Client* hostedClient;

#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#ifndef HOSTED_SERVER_IP
#define REMOTE_IP IpAddress("192.168.13.1")
#else
#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)
#define REMOTE_IP STRINGIFY(HOSTED_SERVER_IP)
#endif

extern "C" void __real__Z4initv();

namespace
{
TcpClient* tcpClient;
Hosted::Transport::TcpClientStream* stream;

void ready(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	if(hostedClient != nullptr) {
		return;
	}

	IpAddress remoteIp(REMOTE_IP);
	if(gateway == IpAddress("192.168.4.1")) {
		remoteIp = gateway;
	}

	tcpClient = new TcpClient(false);
	tcpClient->connect(remoteIp, 4031);
	stream = new Hosted::Transport::TcpClientStream(*tcpClient);

	hostedClient = new Hosted::Client(*stream, '>');
	hostedClient->getRemoteCommands();

	__real__Z4initv();
}

} // namespace

extern "C" void __wrap__Z4initv()
{
	WifiEvents.onStationGotIP(ready);
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	WifiStation.connect();
}
