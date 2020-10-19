#include <SmingCore.h>
#include <Network/UPnP/DeviceHost.h>
#include <Network/SSDP/Server.h>
#include <DeviceFinder.h>
#include <TeaPot.h>
#include <vr900.h>
#include <AV.h>
#include <Wemo.h>
#include <malloc_count.h>

#include "test-desc.h"
extern void testSoap();
extern void testSoap2();

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
HttpServer server;
TeaPot teapot(1);
VR900::GatewayDevice vr900Gateway;
AV::Server avServer;
DeviceFinder deviceFinder;

//static Wemo::SocketList sockets;
Wemo::Controllee wemo1(1, "Socket #1");
Wemo::Controllee wemo2(2, "Socket #2");

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

int onHttpRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(UPnP::deviceHost.onHttpRequest(connection)) {
		return 0;
	}

	Serial.print("Page not found: ");
	Serial.println(request.uri.Path);

	response.code = HTTP_STATUS_NOT_FOUND;
	return 0;
}

void initUPnP()
{
	//	dumpDescription(vr900Gateway);

	server.listen(80);
	server.paths.setDefault(onHttpRequest);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	server.setBodyParser(MIME_XML, bodyToStringParser);

	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return;
	}

	UPnP::deviceHost.registerDevice(&teapot);

	//	UPnP::deviceHost.registerDevice(&vr900Gateway);
	//	UPnP::deviceHost.registerDevice(&avServer);
	//	UPnP::deviceHost.registerDevice(&wemo1);
	//	UPnP::deviceHost.registerDevice(&wemo2);

	UPnP::deviceHost.registerControlPoint(&deviceFinder);

	auto timer = new AutoDeleteTimer;
	timer->initializeMs<1000>(InterruptCallback([]() {
		Serial.println();
		auto ms = new SSDP::MessageSpec(SSDP::MESSAGE_MSEARCH);
		ms->object = &deviceFinder;
		ms->repeat = 2;
		ms->target = SSDP::TARGET_ROOT; // Will get overridden by our custom search handler
		SSDP::server.messageQueue.add(ms, 1000);
	}));
	timer->startOnce();
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	debugf("GotIP: %s", ip.toString().c_str());

	if(ntpClient == nullptr) {
		ntpClient = new NtpClient([](NtpClient& client, time_t timestamp) {
			SystemClock.setTime(timestamp, eTZ_UTC);
			Serial.print("Time synchronized: ");
			Serial.println(SystemClock.getSystemTimeString());
		});
	};

	initUPnP();
}

} // namespace

void init()
{
	Serial.setTxBufferSize(4096);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	//	dumpDescription(&avServer);

#ifdef ARCH_HOST
//	system_restart();
//	return;
#endif

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);

	auto timer = new SimpleTimer;
	timer
		->initializeMs<5000>([]() {
			Serial.print("Free heap: ");
			Serial.print(system_get_free_heap_size());
			Serial.print(", used: ");
			Serial.print(MallocCount::getCurrent());
			Serial.print(", peak: ");
			Serial.println(MallocCount::getPeak());
			MallocCount::resetPeak();
		})
		.start();
}
