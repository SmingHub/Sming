#include <SmingCore.h>
#include <HostedClient.h>
#include "HostedTcpStream.h"

HostedClient* hostedClient{nullptr};

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

extern void init();

static void ready(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	IpAddress remoteIp(REMOTE_IP);
	if(gateway == IpAddress("192.168.4.1")) {
		remoteIp = gateway;
	}

	hostedClient = new HostedClient(new HostedTcpStream(remoteIp.toString(), 4031));
	init();
}

void host_init()
{
	WifiEvents.onStationGotIP(ready);
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	WifiStation.connect();
}
