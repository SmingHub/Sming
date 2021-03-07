#include <SmingCore.h>
#include <Network/Mdns/Server.h>
#include <Network/Mdns/debug.h>
#include <IFS/FileSystem.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

class MessageHandler : public mDNS::Handler
{
public:
	/*
	 * Set the DNS record name we're interested in.
	 * Used as a filter in our `onMessage` method.
	 */
	void setSearchName(const String& name)
	{
		searchName = name;
	}

	bool onMessage(mDNS::Message& message) override;

private:
	String searchName;
};

static MessageHandler myMessageHandler;

#ifdef ARCH_HOST
bool savePacket(IpAddress remoteIP, uint16_t remotePort, const uint8_t* data, size_t length)
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

	return true;
}
#endif

bool MessageHandler::onMessage(mDNS::Message& message)
{
	mDNS::printMessage(Serial, message);

	// Check if we're interested in this message
	if(!message.isReply()) {
		debug_i("Ignoring query");
		return false;
	}
	auto answer = message[mDNS::ResourceType::PTR];
	if(answer == nullptr) {
		debug_i("Ignoring message: no PTR record");
		return false;
	}
	if(answer->getName() != searchName) {
		debug_i("Ignoring message: Name doesn't match");
		return false;
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

	return true;
}

void sendSearch()
{
	String name = F("_googlecast._tcp.local");

	// To discover all DNS-SD registered services, use:
	// name = F("_services._dns-sd._udp.local");

	bool ok = mDNS::server.search(name);
	debug_i("search('%s'): %s", name.c_str(), ok ? "OK" : "FAIL");
	// Tell our handler what we're looking for
	myMessageHandler.setSearchName(name);
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	mDNS::server.onSend([](mDNS::Message& msg) {
		printMessage(Serial, msg);
		return true;
	});
	mDNS::server.addHandler(myMessageHandler);
#ifdef ARCH_HOST
	mDNS::server.onPacket(savePacket);
#endif

	// Issue a search now
	sendSearch();

	// Repeat every 30 seconds
	auto timer = new Timer;
	timer->initializeMs<30000>(sendSearch);
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
		mDNS::printMessage(Serial, message);
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
		Query query;
		query.addQuestion(F("_chromecast._tcp.local"));
		query.addQuestion(F("_%9832479817234_sming._tcp.local"), mDNS::ResourceType::PTR);
		query.addQuestion(F("_sming._tcp.local"), mDNS::ResourceType::PTR);
		parseFile(F("Test questions"), query);
	}

	// Create message records
	{
		/*
		 * Service is "_test", hostname is "sming"
		 *
		 * We also demonstrate here how to set up a name pointer.
		*/
		Request reply(Request::Type::reply);

		/**
		 * Anywhere a Name is used, if the final character is '.' then it is stored as a 'pointer',
		 * a 16-bit offset into the message data. We don't know the value of that pointer yet, so a
		 * placeholder is inserted.
		 * 
		 * In this case, we want our PTR record to reference the name of the SRV record which we add later.
		 */
		auto ptr = reply.addAnswer<Resource::PTR>(F("_test._tcp.local"), ".");

		// Move to 'nameserver' records section
		reply.nextSection();
		// Move to 'additional' records section
		reply.nextSection();

		/**
		 * We'll use a pointer for the name here, saves space.
		 */
		auto txt = reply.addAnswer<Resource::TXT>(".");
		txt.add("pm=12");
		txt.add("fn=My friendly name");
		auto srv = reply.addAnswer<Resource::SRV>(F("sming._test._tcp.local"), 1, 2, 8080, F("sming.local"));

		/**
		 * Now the SRV record is constructed we can fix our name references.
		 * The 'fixup' call will essentially replace the final '.' we added above with a pointer to the actual name.
		 * Note that the call will fail if we didn't append that final '.', since there will be nowhere to write
		 * the pointer value.
		 */
		auto serviceName = srv.answer.getName();
		ptr.getName().fixup(serviceName);
		txt.answer.getName().fixup(serviceName);

		/**
		 * Finally, write the address records
		 * We can re-use the host Name from the SRV record, storing a pointer instead of the full text
		 */
		auto hostName = srv.getHost();
		reply.addAnswer<Resource::A>(hostName, WifiStation.getIP());
		reply.addAnswer<Resource::AAAA>(hostName, Ip6Address());

		parseFile(F("Test answers"), reply);
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
