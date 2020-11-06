#include <SmingCore.h>
#include <Network/UPnP/DeviceHost.h>
#include <Dial/Client.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

static Dial::Client client;

void onRun(Dial::App& app, HttpResponse& response)
{
	if(response.isSuccess()) {
		auto timer = new AutoDeleteTimer;
		timer->initializeMs<20000>([&]() {
			// Once started the app can also be stopped using the command below
			Serial.print(_F("Stopping application: "));
			Serial.println(app.getName());
			app.stop();
		});
		timer->startOnce();
	}
}

void onStatus(Dial::App& app, HttpResponse& response)
{
	if(!response.isSuccess()) {
		Serial.print(_F("Error locating '"));
		Serial.print(app.getName());
		Serial.print(_F("' application: "));
		Serial.println(toString(response.code));
		return;
	}

	HttpParams params;
	params["v"] = "fC9HdQUaFtA";
	app.run(params, onRun);
}

void onConnected(Dial::Client& client, HttpConnection& connection, const XML::Document& description)
{
	Serial.println(_F("New DIAL device found: "));

	auto node = XML::getNode(description, F("/device/friendlyName"));
	if(node == nullptr) {
		Serial.println(_F("UNEXPECTED! friendlyName missing from device description"));
	} else {
		Serial.print(_F("Friendly name '"));
		Serial.print(node->value());
		Serial.println('\'');
	}

#if DEBUG_VERBOSE_LEVEL == DBG
	Serial.println();
	XML::serialize(description, Serial, true);
	Serial.println();
#endif

	auto& app = client.getApp("YouTube");
	app.status(onStatus);
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(_F("I'm CONNECTED to "));
	Serial.println(ip);

	if(!UPnP::deviceHost.begin()) {
		debug_e("UPnP initialisation failed");
		return;
	}

	/* The command below will use UPnP to auto-discover a smart monitor/TV */
	client.connect(onConnected);

	/* Alternatevely one can use the commands below when auto-discovery is not working */
	/*
	Url descriptionUrl{"http://192.168.22.222:55000/nrc/ddd.xml"};

	client.connect(descriptionUrl, onConnected);
	*/
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	// The different reason codes can be found in user_interface.h. in your SDK.
	Serial.print(_F("Disconnected from \""));
	Serial.print(ssid);
	Serial.print(_F("\", reason: "));
	Serial.println(WifiEvents.getDisconnectReasonDesc(reason));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	// Station - WiFi client
	WifiStation.enable(true);
	WifiStation.config(F(WIFI_SSID), F(WIFI_PWD));

	// Set callback that should be triggered when we have assigned IP
	WifiEvents.onStationGotIP(connectOk);

	// Set callback that should be triggered if we are disconnected or connection attempt failed
	WifiEvents.onStationDisconnect(connectFail);
}
