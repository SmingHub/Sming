#include <SmingCore.h>
#include <Dial/Client.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
Dial::Client client;

void onRun(Dial::App& app, HttpResponse& response)
{
	if(response.isSuccess()) {
		auto timer = new AutoDeleteTimer;
		timer->initializeMs<20000>([&]() {
			// Once started the app can also be stopped using the command below
			Serial.printf(_F("Stopping application: %s\n"), app.getName().c_str());
			app.stop();
		});
		timer->startOnce();
	}
}

void onStatus(Dial::App& app, HttpResponse& response)
{
	if(!response.isSuccess()) {
		Serial.printf(_F("Unable to find the desired application: %s\n"), app.getName().c_str());
		return;
	}

	HttpParams params;
	params["v"] = "fC9HdQUaFtA";
	app.run(params, onRun);
}

void onConnected(Dial::Client& client, const XML::Document& doc, const HttpHeaders& headers)
{
	auto node = XML::getNode(doc, "/device/friendlyName");
	Serial.println(_F("New DIAL device found."));
	if(node != nullptr) {
		Serial.printf(_F("Friendly name: %s.\n"), node->value());
	}

	auto& app = client.getApp("YouTube");
	app.status(onStatus);
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(_F("I'm CONNECTED to "));
	Serial.println(ip);

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

} // namespace

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
