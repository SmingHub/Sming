#include <SmingCore.h>
#include <Network/Mdns/Finder.h>
#include <Network/Mdns/Request.h>
#include <IFS/FileSystem.h>

IMPORT_FSTR(testFile, PROJECT_DIR "/resource/192.168.1.100.mdns")

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

mDNS::Finder finder;

DEFINE_FSTR_LOCAL(fstrSearchInstance, "_googlecast")

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

void print(const String& tag, const String& value)
{
	String s = "   ";
	s += tag;
	s += ':';
	while(s.length() < 12) {
		s += ' ';
	}
	Serial.print(s);
	Serial.print(' ');
	Serial.println(value);
}

void printHex(const String& tag, uint16_t value)
{
	char buf[10];
	m_snprintf(buf, sizeof(buf), "0x%04x", value);
	print(tag, buf);
}

void printBool(const String& tag, bool value)
{
	print(tag, value ? "Y" : "N");
}

void printQuestion(mDNS::Question& question)
{
	Serial.println(F(">> Question"));
	print(F("name"), question.getName());
	auto type = question.getType();
	print(F("type"), toString(type));
	printHex(F("type"), uint16_t(type));
	printHex(F("class"), question.getClass());
	printBool(F("unicast"), question.isUnicastResponse());
}

void printAnswer(mDNS::Answer& answer)
{
	Serial.print(">> ");
	Serial.println(toString(answer.getKind()));
	print(F("name"), answer.getName());
	print(F("data"), answer.getRecordString());
	auto type = answer.getType();
	print(F("type"), toString(type));
	printHex(F("type"), uint16_t(type));
	print(F("ttl"), String(answer.getTtl()));
	printHex(F("class"), answer.getClass());
	printBool(F("flush"), answer.isCachedFlush());
}

void printResponse(mDNS::Response& response)
{
	Serial.println();
	Serial.print(response.isAnswer() ? F("RESPONSE") : F("REQUEST"));
	auto ip = response.getRemoteIp();
	if(uint32_t(ip) != 0) {
		Serial.print(F(" from "));
		Serial.print(response.getRemoteIp().toString());
		Serial.print(':');
		Serial.println(response.getRemotePort());
	} else {
		Serial.println();
	}

	Serial.print(F("Size: "));
	Serial.print(response.getSize());
	Serial.println(F(" bytes"));

	for(auto& question : response.questions) {
		printQuestion(question);
	}

	for(auto& answer : response.answers) {
		printAnswer(answer);
	}
}

void handleResponse(mDNS::Response& response)
{
	printResponse(response);

	// Check if we're interested in this reponse
	if(!response.isAnswer()) {
		return;
	}
	auto answer = response[mDNS::ResourceType::PTR];
	if(answer == nullptr) {
		return;
	}
	if(answer->getName().getInstance() != fstrSearchInstance) {
		return;
	}

	// Extract our required information from the response
	struct {
		String manufacturerDevice;
		String friendlyName;
		IpAddress ipaddr;
	} info;

	answer = response[mDNS::ResourceType::TXT];
	if(answer != nullptr) {
		mDNS::Resource::TXT txt(*answer);
		info.manufacturerDevice = txt["md"];
		info.friendlyName = txt["fn"];
	}

	answer = response[mDNS::ResourceType::A];
	if(answer != nullptr) {
		mDNS::Resource::A a(*answer);
		info.ipaddr = a.getAddress();
	}

	Serial.print(F("Manufacturer Device = '"));
	Serial.print(info.manufacturerDevice);
	Serial.print(F("', Friendly Name = '"));
	Serial.print(info.friendlyName);
	Serial.print(F("', IP Address = "));
	Serial.println(info.ipaddr);
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	finder.onAnswer(handleResponse);
#ifdef ARCH_HOST
	finder.onPacket(savePacket);
#endif

	auto timer = new Timer;
	timer->initializeMs<10000>(InterruptCallback([]() {
		bool ok = finder.search(String(fstrSearchInstance) + F("._tcp.local"));
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
	debug_i("sizeof(mDNS::Question) = %u", sizeof(mDNS::Question));
	debug_i("sizeof(mDNS::Answer) = %u", sizeof(mDNS::Answer));
	debug_i("sizeof(LinkedObject) = %u", sizeof(LinkedObject));

	using namespace mDNS;

	// Create list of questions, serialise them then decode and print the result
	{
		Request request(Request::Type::query);
		request.addQuestion(F("_chromecast._tcp.local"));
		request.addQuestion(F("_%9832479817234_sming._tcp.local"), mDNS::ResourceType::PTR);
		request.addQuestion(F("_sming._tcp.local"), mDNS::ResourceType::PTR);
		Response response(0U, 0, request.getData(), request.getSize());
		response.parse();
		printResponse(response);
	}

	// Create response records
	{
		Request request(Request::Type::reply);
		auto ptr = request.addAnswer<Resource::PTR>(F("_chromecast._tcp.local"), F("my.test.name._tcp.local"));
		request.nextSection();
		request.nextSection();
		auto a = request.addAnswer<Resource::A>(F("my.test.name._tcp.local"), 0x12345678);
		auto txt = request.addAnswer<Resource::TXT>(F("my.test.name._tcp.local"));
		txt.add("pm=12");
		txt.add("fn=My friendly name");
		auto aaaa = request.addAnswer<Resource::AAAA>(F("abc._tcp.local"), Ip6Address());
		auto srv = request.addAnswer<Resource::SRV>(F("xxxxx._tcp.local"), 1, 2, 8080, F("sillyhouse.net"));
		printResponse(request);
	}

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
