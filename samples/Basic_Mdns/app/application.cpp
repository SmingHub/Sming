#include <SmingCore.h>
#include <Network/Mdns/Server.h>
#include <IFS/FileSystem.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

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
	printBool(F("unicast"), question.isUnicastReply());
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

void printMessage(mDNS::Message& message)
{
	Serial.println();
	Serial.print(system_get_time());
	Serial.print(' ');
	Serial.print(message.isReply() ? F("REPLY") : F("QUERY"));
	auto ip = message.getRemoteIp();
	if(uint32_t(ip) != 0) {
		Serial.print(F(" from "));
		Serial.print(message.getRemoteIp().toString());
		Serial.print(':');
		Serial.println(message.getRemotePort());
	} else {
		Serial.println();
	}

	Serial.print(F("Size: "));
	Serial.print(message.getSize());
	Serial.println(F(" bytes"));

	for(auto& question : message.questions) {
		printQuestion(question);
	}

	for(auto& answer : message.answers) {
		printAnswer(answer);
	}
}

void handleMessage(mDNS::Message& message)
{
	printMessage(message);

	// Check if we're interested in this reponse
	if(!message.isReply()) {
		return;
	}
	auto answer = message[mDNS::ResourceType::PTR];
	if(answer == nullptr) {
		return;
	}
	if(answer->getName().getInstance() != fstrSearchInstance) {
		return;
	}

	// Extract our required information from the message
	struct {
		String manufacturerDevice;
		String friendlyName;
		IpAddress ipaddr;
	} info;

	answer = message[mDNS::ResourceType::TXT];
	if(answer != nullptr) {
		mDNS::Resource::TXT txt(*answer);
		info.manufacturerDevice = txt["md"];
		info.friendlyName = txt["fn"];
	}

	answer = message[mDNS::ResourceType::A];
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

	mDNS::server.onMessage(handleMessage);
#ifdef ARCH_HOST
	mDNS::server.onPacket(savePacket);
#endif

	auto timer = new Timer;
	timer->initializeMs<10000>(InterruptCallback([]() {
		bool ok = mDNS::server.search(String(fstrSearchInstance) + F("._tcp.local"));
		debug_i("search(): %s", ok ? "OK" : "FAIL");
	}));
	timer->start();
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(F("I'm NOT CONNECTED!"));
}

void parseFile(const String& name, const void* data, size_t length)
{
	Serial.println();
	Serial.print(_F("** Parsing '"));
	Serial.print(name);
	Serial.println(_F("' **"));
	mDNS::Message message(0U, 0, const_cast<void*>(data), length);
	if(message.parse()) {
		printMessage(message);
	}
	Serial.println(_F("** End of test packet **"));
	Serial.println();
}

void parseFile(const String& name, const String& data)
{
	parseFile(name, data.c_str(), data.length());
}

void parseFile(const String& name, const mDNS::Message& message)
{
	parseFile(name, message.getData(), message.getSize());
}

void test()
{
	debug_i("sizeof(mDNS::Server) = %u", sizeof(mDNS::Server));
	debug_i("sizeof(mDNS::Message) = %u", sizeof(mDNS::Message));
	debug_i("sizeof(mDNS::Question) = %u", sizeof(mDNS::Question));
	debug_i("sizeof(mDNS::Answer) = %u", sizeof(mDNS::Answer));
	debug_i("sizeof(LinkedObject) = %u", sizeof(LinkedObject));

	using namespace mDNS;

	// Create list of questions, parse the resulting data and print the result
	{
		Request request(Request::Type::query);
		request.addQuestion(F("_chromecast._tcp.local"));
		request.addQuestion(F("_%9832479817234_sming._tcp.local"), mDNS::ResourceType::PTR);
		request.addQuestion(F("_sming._tcp.local"), mDNS::ResourceType::PTR);
		parseFile(F("Test questions"), request);
	}

	// Create message records
	{
		// Service is "_test", hostname is "sming"
		Request request(Request::Type::reply);
		auto ptr = request.addAnswer<Resource::PTR>(F("_test._tcp.local"), F("sming._test._tcp.local"));
		request.nextSection();
		request.nextSection();
		auto txt = request.addAnswer<Resource::TXT>(F("sming._test._tcp.local"));
		txt.add("pm=12");
		txt.add("fn=My friendly name");
		auto srv = request.addAnswer<Resource::SRV>(F("sming._test._tcp.local"), 1, 2, 8080, F("sming.local"));
		auto a = request.addAnswer<Resource::A>(F("sming.local"), WifiStation.getIP());
		auto aaaa = request.addAnswer<Resource::AAAA>(F("sming.local"), Ip6Address());
		parseFile(F("Test answers"), request);
	}

// For host, read the resource directory directly as we might want to add other files there
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
#endif
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	test();

	// Setup the WIFI connection
	// IMPORTANT: MUST disable AP or multicast won't work
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
