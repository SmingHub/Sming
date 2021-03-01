#include <SmingCore.h>
#include <Network/Mdns/Finder.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

mDNS::Finder finder;

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	finder.onAnswer([](mDNS::Answer& answer) {
		// m_printHex("ANSWER", &answer, sizeof(answer));
		debug_i(">> name:  %s", answer.name);
		debug_i("   data:  %s", answer.data);
		debug_i("   type:  0x%04x", answer.type);
		debug_i("   class: 0x%04x", answer.klass);
		debug_i("   ttl:   %u", answer.ttl);
		debug_i("   flsh?: %u", answer.isCachedFlush);
		debug_i("   vald?: %u", answer.isValid);
	});
	// bool ok = finder.search("_googlecast._tcp.local");
	String hostname = F("_googlecast._tcp.local");
	mDNS::Query query{};
	memcpy(query.name, hostname.c_str(), hostname.length());
	query.type = mDNS::MDNS_TYPE_PTR;
	query.klass = 1; // "INternet"
	query.isUnicastResponse = false;
	query.isValid = true;
	bool ok = finder.search(query);

	debug_i("search(): %s", ok ? "OK" : "FAIL");
	// finder.search("googlecast._tcp.local");
	// finder.search("_googlecast");
	// finder.search("googlecast");
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(F("I'm NOT CONNECTED!"));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
