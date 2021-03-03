#include <SmingCore.h>
#include <Network/Mdns/Finder.h>
#include <IFS/FileSystem.h>

IMPORT_FSTR(testFile, PROJECT_DIR "/resource/192.168.1.100.mdns")

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

mDNS::Finder finder;

#ifdef ARCH_HOST
void savePacket(IpAddress remoteIP, uint16_t remotePort, const uint8_t* data, size_t length)
{
	auto& hostfs = IFS::Host::getFileSystem();
	String filename;
	filename = "out/mdns/";
	hostfs.makedirs(filename);
	filename += DateTime(SystemClock.now()).toISO8601();
	filename += '-';
	filename += remoteIP.toString();
	filename += '-';
	filename += remotePort;
	filename += ".bin";
	filename.replace(':', '-');
	hostfs.setContent(filename, data, length);
}
#endif

void printQuestion(mDNS::Question& question)
{
	Serial.println(F(">> Question"));

	auto name = question.getName();
	Serial.print(F("  name: "));
	Serial.println(name);

	Serial.print(F("  instance: "));
	Serial.println(name.getInstance());
	Serial.print(F("  service:  "));
	Serial.println(name.getService());
	Serial.print(F("  domain:   "));
	Serial.println(name.getDomain());

	Serial.print(F("  type:  "));
	auto type = question.getType();
	Serial.print(toString(type));
	Serial.print(F(" (0x"));
	Serial.print(unsigned(type), HEX);
	Serial.println(")");

	Serial.print(F("  class: 0x"));
	Serial.println(question.getClass(), HEX);
	Serial.print(F("  ucast: "));
	Serial.println(question.isUnicastResponse());
}

void printAnswer(mDNS::Answer& answer)
{
	debug_i(">> name:  %s", String(answer.getName()).c_str());
	debug_i("   data:  %s", answer.getRecordString().c_str());
	debug_i("   type:  %s (0x%04X)", toString(answer.getType()).c_str(), unsigned(answer.getType()));
	debug_i("   class: 0x%04x", answer.getClass());
	debug_i("   ttl:   %u", answer.getTtl());
	debug_i("   flush: %u", answer.isCachedFlush());
}

void printResponse(mDNS::Response& response)
{
	Serial.println();
	Serial.print(response.isAnswer() ? F("REQUEST") : F("RESPONSE"));
	auto ip = response.getRemoteIp();
	if(uint32_t(ip) != 0) {
		Serial.print(F(" from "));
		Serial.print(response.getRemoteIp().toString());
		Serial.print(':');
		Serial.println(response.getRemotePort());
	} else {
		Serial.println();
	}

	for(auto& question : response.questions) {
		printQuestion(question);
	}

	for(auto& answer : response.answers) {
		printAnswer(answer);
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
#ifdef ARCH_HOST
	finder.onPacket(savePacket);
#endif

	auto timer = new Timer;
	timer->initializeMs<10000>(InterruptCallback([]() {
		bool ok = finder.search(F("_googlecast._tcp.local"));
		debug_i("search(): %s", ok ? "OK" : "FAIL");
	}));
	timer->start();
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(F("I'm NOT CONNECTED!"));
}

void parseFile(const String& name, const String& data)
{
	Serial.println();
	Serial.print(_F("** Parsing '"));
	Serial.print(name);
	Serial.println(_F("' **"));
	mDNS::Response response(0U, 0, const_cast<char*>(data.begin()), data.length());
	if(response.parse()) {
		printResponse(response);
	}
	Serial.println(_F("** End of test packet **"));
	Serial.println();
}

void test()
{
	debug_i("sizeof(mDNS::Finder) = %u", sizeof(mDNS::Finder));
	debug_i("sizeof(mDNS::Response) = %u", sizeof(mDNS::Response));
	debug_i("sizeof(mDNS::Answer) = %u", sizeof(mDNS::Answer));
	debug_i("sizeof(LinkedObject) = %u", sizeof(LinkedObject));

#ifdef ARCH_HOST

	auto& fs = IFS::Host::getFileSystem();
	IFS::Directory dir(&fs);
	if(dir.open("resource")) {
		while(dir.next()) {
			String filename = dir.getDirName() + "/" + String(dir.stat().name);
			String data(fs.getContent(filename));
			parseFile(filename, data);
		}
	}

#else
	parseFile(F("testFile"), testFile);
#endif
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
