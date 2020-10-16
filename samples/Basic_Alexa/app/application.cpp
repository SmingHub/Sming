#include <SmingCore.h>
#include <UPnP/DeviceHost.h>
#include <Hue/Bridge.h>
#include <Hue/DeviceList.h>
#include <malloc_count.h>

static HttpServer server;

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

/*
 * TODO: We should be able to use UPnP to manage our device list
 * as linked items, then we can get rid of DeviceList completely.
 */
static Hue::DeviceList devices;
static Hue::DeviceListEnumerator enumerator(devices);
static Hue::Bridge bridge(enumerator);

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

static int onHttpRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
{
	if(UPnP::deviceHost.onHttpRequest(connection)) {
		return 0;
	}

	auto path = request.uri.getRelativePath();
	if(path.length() == 0 || path.equalsIgnoreCase(F("index.html"))) {
		response.sendString(F("<html>"
							  "<h1>Welcome to Sming</h1>"
							  "Basic_Alexa Demo"
							  "</html>"));
		return 0;
	}

	Serial.print("Page not found: ");
	Serial.println(request.uri.Path);

	response.code = HTTP_STATUS_NOT_FOUND;
	return 0;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	debugf("GotIP: %s", ip.toString().c_str());

	server.listen(bridge.getTcpPort());
	server.paths.setDefault(onHttpRequest);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	server.setBodyParser(MIME_XML, bodyToStringParser);

	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return;
	}

	UPnP::deviceHost.registerDevice(&bridge);

	auto Name = [](unsigned id) -> String {
		String s;
#ifdef ARCH_HOST
		s += "Host";
#else
		s += "Esp";
#endif
		s += '-';
		s += id;
		return s;
	};

	devices.addElement(new Hue::OnOffDevice(101, Name(101)));
	devices.addElement(new Hue::DimmableDevice(102, Name(102)));
	devices.addElement(new Hue::ColourDevice(103, Name(103)));

	devices.addElement(new Hue::OnOffDevice(253, Name(253)));
	devices.addElement(new Hue::DimmableDevice(254, Name(254)));
	devices.addElement(new Hue::ColourDevice(255, Name(255)));

	devices.addElement(new Hue::OnOffDevice(256, Name(256)));

	devices.addElement(new Hue::OnOffDevice(500, Name(500)));
	devices.addElement(new Hue::DimmableDevice(501, Name(501)));
	devices.addElement(new Hue::ColourDevice(502, Name(502)));

	devices.addElement(new Hue::ColourDevice(54321, Name(54321)));
	devices.addElement(new Hue::ColourDevice(0x7FFFFFFF, Name(0x7FFFFFFF)));

	bridge.onStateChanged([](const Hue::Device& device, unsigned attr) {
		debug_i("#%u %s changed 0x%08x", device.getId(), device.getName().c_str(), attr);
	});
}

void init()
{
	Serial.setTxBufferSize(4096);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	WifiStation.enable(true, false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiAccessPoint.enable(false, false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);

	auto timer = new SimpleTimer;
	timer->initializeMs<5000>([]() {
		Serial.print("Free heap: ");
		Serial.print(system_get_free_heap_size());
		Serial.print(", used: ");
		Serial.print(MallocCount::getCurrent());
		Serial.print(", peak: ");
		Serial.println(MallocCount::getPeak());
		MallocCount::resetPeak();
	});
	timer->start();
}
