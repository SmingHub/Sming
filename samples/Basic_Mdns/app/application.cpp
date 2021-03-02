#include <SmingCore.h>
#include <Network/Mdns/Finder.h>

IMPORT_FSTR(testFile, PROJECT_DIR "/resource/192.168.1.100.mdns")

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

mDNS::Finder finder;

void printResponse(mDNS::Response& response)
{
	Serial.println();
	debug_i("RESPONSE!!");
	for(auto& answer : response) {
		debug_i(">> name:  %s", String(answer.getName()).c_str());
		debug_i("   data:  %s", answer.getRecordString().c_str());
		debug_i("   type:  %s (0x%04X)", toString(answer.getType()).c_str(), unsigned(answer.getType()));
		debug_i("   class: 0x%04x", answer.getClass());
		debug_i("   ttl:   %u", answer.getTtl());
		debug_i("   flush: %u", answer.isCachedFlush());
	}

	auto answer = response[mDNS::ResourceType::TXT];
	if(answer != nullptr) {
		mDNS::Resource::TXT txt(*answer);
		auto s = txt["md"];
		debug_i("md = '%s'", s.c_str());
	}

	answer = response[mDNS::ResourceType::A];
	if(answer != nullptr) {
		mDNS::Resource::A a(*answer);
		debug_i("addr = %s", a.toString().c_str());
	}
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	finder.onAnswer(printResponse);

	// bool ok = finder.search("_googlecast._tcp.local");
	mDNS::Query query{};
	query.name = F("_googlecast._tcp.local");
	query.type = mDNS::ResourceType::PTR;
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

void test()
{
	debug_i("sizeof(mDNS::Finder) = %u", sizeof(mDNS::Finder));
	debug_i("sizeof(mDNS::Response) = %u", sizeof(mDNS::Response));
	debug_i("sizeof(mDNS::Answer) = %u", sizeof(mDNS::Answer));
	debug_i("sizeof(LinkedObject) = %u", sizeof(LinkedObject));

	Serial.println(_F("** Parsing test packet **"));
	String data(testFile);
	mDNS::Response response(data.begin(), data.length());
	response.parse();
	printResponse(response);
	Serial.println(_F("** End of test packet **"));
	Serial.println();
	Serial.println();
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	test();

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
