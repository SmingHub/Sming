#include <SmingCore.h>
#include <Network/UPnP/DeviceHost.h>
#include <Hue/Bridge.h>
#include <Hue/DeviceList.h>
#include <Hue/ColourDevice.h>
#include <MyHueDevice.h>
#include <malloc_count.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
NtpClient* ntpClient;
HttpServer server;
Hue::DeviceList devices;
Hue::DeviceListEnumerator enumerator(devices);
Hue::Bridge bridge(enumerator);

constexpr uint8_t LED_PIN{2}; // GPIO2

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

int onHttpRequest(HttpServerConnection& connection, HttpRequest& request, HttpResponse& response)
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

	if(ntpClient == nullptr) {
		ntpClient = new NtpClient([](NtpClient& client, time_t timestamp) {
			SystemClock.setTime(timestamp, eTZ_UTC);
			Serial.print("Time synchronized: ");
			Serial.println(SystemClock.getSystemTimeString());
		});
	};

	server.listen(bridge.getTcpPort());
	server.paths.setDefault(onHttpRequest);
	server.setBodyParser(MIME_JSON, bodyToStringParser);
	server.setBodyParser(MIME_XML, bodyToStringParser);

	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return;
	}

	UPnP::deviceHost.registerDevice(&bridge);

	/*
	 * To avoid confusion when testing with a Host or real device, ensure lighting devices are
	 * created with different names so they can be distinguished in the Alexa App.
	 */
	auto Name = [](unsigned id) -> String {
		String s;
		s += MACROQUOTE(SMING_ARCH);
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

	devices.addElement(new MyHueDevice(666, F("My Custom Hue Device")));

	// Connect the LED pin to light 101
	pinMode(LED_PIN, OUTPUT);
	digitalWrite(LED_PIN, HIGH); // Turn off - state is inverted

	// Monitor when device attributes change
	bridge.onStateChanged([&](const Hue::Device& device, Hue::Device::Attributes attr) {
		debug_i("#%u %s changed [%s]", device.getId(), device.getName().c_str(), toString(attr).c_str());

		/*
		 * Control the LED pin
		 */
		using Attr = Hue::Device::Attribute;
		if(device.getId() == 101 && attr[Attr::on]) {
			unsigned value;
			if(device.getAttribute(Attr::on, value)) {
				digitalWrite(LED_PIN, value == 0);
			}
		}
	});

	// Monitor and handle requests to authorize or revoke users
	bridge.onConfigChange([&](const Hue::Bridge::Config& config) {
		debug_i("%s: deviceType=%s, name=%s",
				config.type == Hue::Bridge::Config::Type::AuthorizeUser ? "Authorize" : "Revoke",
				config.deviceType.c_str(), config.name.c_str());
	});

	/*
	 * Allow creation of users.
	 *
	 * WARNING! Security risk. In a real application, you would only enable pairing
	 * on request from a secure web-page or by pressing a button. In addition, a suitable
	 * timeout should be applied (e.g. 1 minute).
	 *
	 */
	bridge.enablePairing(true);
}

} // namespace

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
