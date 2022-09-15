#include <SmingCore.h>
#include <Network/Mdns/Responder.h>
#include <Network/Mdns/debug.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

// Our device will be reachable via "sming.local"
DEFINE_FSTR_LOCAL(hostName, "sming");

HttpServer server;

class MyHttpService : public mDNS::Service
{
public:
	String getInstance() override
	{
		return F("UDP Server");
	}

	void addText(mDNS::Resource::TXT& txt) override
	{
		txt.add(F("md=Sming Device"));
		txt.add(F("fn=My happy little HTTP service"));
	}
};

static mDNS::Responder responder;
static MyHttpService myHttpService;

void speedTest(mDNS::Question* question)
{
	using namespace mDNS;

	OneShotFastUs timer;
	unsigned count{0};
	for(unsigned i = 0; i < 10000; ++i) {
		if(question->getName() == fstrServicesLocal) {
			++count;
		}
		if(question->getName().equalsIgnoreCase("Haggis basher")) {
			++count;
		}
	}
	debug_i("(question == fstrServicesLocal): %u, %s", count, timer.elapsedTime().toString().c_str());

	timer.start();
	count = 0;
	for(unsigned i = 0; i < 10000; ++i) {
		if(String(question->getName()).equalsIgnoreCase(fstrServicesLocal)) {
			++count;
		}
		if(String(question->getName()).equalsIgnoreCase("Haggis basher")) {
			++count;
		}
	}
	debug_i("(question == fstrServicesLocal): %u, %s", count, timer.elapsedTime().toString().c_str());
}

void test()
{
	using namespace mDNS;

	mDNS::server.onSend([](Message& msg) {
		printMessage(Serial, msg);
		return false;
	});

	auto submit = [&](const String& name, ResourceType type) {
		Query query;
		auto question = query.addQuestion(name, type);
		printMessage(Serial, query);
		responder.onMessage(query);
	};

	auto checkLike = [](Question* question, const char* s, bool isLike) {
		auto name = question->getName();
		bool match = (name == s);
		debug_i("(question == \"%s\"): %u", s, match);
		// TEST_ASSERT(match == isLike);
	};

	auto checkParser = [](const char* s) {
		Query query;
		auto question = query.addQuestion(s);
		auto name = question->getName();
		debug_i("instance: '%s'", String(name).c_str());
		debug_i("service:  '%s'", String(name.getService()).c_str());
		debug_i("protocol: '%s'", String(name.getProtocol()).c_str());
		debug_i("domain:   '%s'", String(name.getDomain()).c_str());
	};

	{
		Query query;
		auto question = query.addQuestion(fstrServicesLocal, ResourceType::PTR);

		// speedTest(question);

		checkLike(question, _F("_services._dns-sd._udp.local"), true);
		checkLike(question, _F("_dns-sd._udp.local"), true);
		checkLike(question, _F("_udp.local"), true);
		checkLike(question, _F("local"), true);

		checkParser(_F("_some-service-or-other._http._tcp.sming.local"));

		// debug_i("(question == fstrServicesLocal): %u", question->getName() == fstrServicesLocal);
		printMessage(Serial, query);
		responder.onMessage(query);
	}

	{
		Query query;
		FSTR_ARRAY(bigYin, "I have instances in my name.with.dots.and.everything._dns-sd._udp.local");
		auto question = query.addQuestion(bigYin, ResourceType::PTR);
		checkLike(question, "everything._dns-sd._udp.local", false);
		checkLike(question, "_dns-sd._udp.local", true);
	}

	auto inst = myHttpService.getInstanceName();
	submit(inst, ResourceType::PTR);
	submit(inst, ResourceType::A);
	submit(inst, ResourceType::TXT);
	submit(inst, ResourceType::SRV);

	mDNS::server.onSend(nullptr);
}

void startmDNS()
{
	responder.begin(hostName);
	responder.addService(myHttpService);

	test();
}

void onIndex(HttpRequest& request, HttpResponse& response)
{
	response.sendFile("index.html");
}

void onFile(HttpRequest& request, HttpResponse& response)
{
	String file = request.uri.getRelativePath();

	if(file[0] == '.') {
		response.code = HTTP_STATUS_FORBIDDEN;
	} else {
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void startWebServer()
{
	server.listen(80);
	server.paths.set("/", onIndex);

	Serial.println(_F("\r\n"
					  "=== WEB SERVER STARTED ==="));
	Serial.println(WifiStation.getIP());
	Serial.println(_F("==========================\r\n"));
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	if(!fileExist("index.html")) {
		String content = F("<h3>Congrats !! You are Connected to your ESP module with mDNS address %host%.local</h3>");
		content.replace(F("%host%"), hostName);
		fileSetContent("index.html", content);
	}
	startWebServer();
	startmDNS(); // Start mDNS "Advertise" of your hostname "test.local" for this example
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Enable debug output to serial

	spiffs_mount();

	WifiStation.config(WIFI_SSID, WIFI_PWD);
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);

	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
