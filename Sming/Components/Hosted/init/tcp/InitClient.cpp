#include <SmingCore.h>
#include <Hosted/Client.h>
#include <Hosted/Transport/TcpClientStream.h>

Hosted::Client* hostedClient{nullptr};

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

namespace
{
TcpClient* tcpClient = nullptr;
Hosted::Transport::TcpClientStream* stream = nullptr;

static void ready(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	if(hostedClient != nullptr) {
		return;
	}

	IpAddress remoteIp(REMOTE_IP);
	if(gateway == IpAddress("192.168.4.1")) {
		remoteIp = gateway;
	}

	tcpClient = new TcpClient(false);
	tcpClient->connect(remoteIp.toString(), 4031);
	stream = new Hosted::Transport::TcpClientStream(*tcpClient);

	hostedClient = new Hosted::Client(*stream);
	hostedClient->getRemoteCommands();
	init();
}

} // namespace

void host_init()
{
	WifiEvents.onStationGotIP(ready);
	WifiStation.enable(true);
	WifiStation.config(_F(WIFI_SSID), _F(WIFI_PWD));
	WifiStation.connect();
}
