#include <SmingCore.h>
#include <CounterStream.h>
#include <Platform/Timers.h>

#ifdef ENABLE_MALLOC_COUNT
#include <malloc_count.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;
HttpClient downloadClient;
OneShotFastMs connectTimer;

void printHeap()
{
	Serial.println(_F("Heap statistics"));
	Serial.print(_F("  Free bytes:  "));
	Serial.println(system_get_free_heap_size());
#ifdef ENABLE_MALLOC_COUNT
	Serial.print(_F("  Used:        "));
	Serial.println(MallocCount::getCurrent());
	Serial.print(_F("  Peak used:   "));
	Serial.println(MallocCount::getPeak());
	Serial.print(_F("  Allocations: "));
	Serial.println(MallocCount::getAllocCount());
	Serial.print(_F("  Total used:  "));
	Serial.println(MallocCount::getTotal());
#endif
}

int onDownload(HttpConnection& connection, bool success)
{
	auto elapsed = connectTimer.elapsedTime();

	Serial.print(_F("Got response code: "));
	auto status = connection.getResponse()->code;
	Serial.print(status);
	Serial.print(" (");
	Serial.print(httpGetStatusText(status));
	Serial.print(_F("), success: "));
	Serial.print(success);

	auto stream = connection.getResponse()->stream;
	assert(stream != nullptr);

	Serial.print(_F(", received "));
	Serial.print(stream->available());
	Serial.println(_F(" bytes"));

	auto& headers = connection.getResponse()->headers;
	for(unsigned i = 0; i < headers.count(); ++i) {
		Serial.print(headers[i]);
	}

	auto ssl = connection.getSsl();
	if(ssl != nullptr) {
		ssl->printTo(Serial);
	}

	Serial.print(_F("Time to connect and download page: "));
	Serial.println(elapsed.toString());

	return 0; // return 0 on success in your callbacks
}

/*
 * Initialise SSL session parameters for connecting to the GRC web server
 */
void grcSslInit(Ssl::Session& session, HttpRequest& request)
{
	debug_i("Initialising SSL session for GRC");

	// Use the Gibson Research fingerprints web page as an example. Unlike Google, these fingerprints change very infrequently.
	static const Ssl::Fingerprint::Cert::Sha1 sha1Fingerprint PROGMEM = {
		0x7A, 0x85, 0x1C, 0xF0, 0xF6, 0x9F, 0xD0, 0xCC, 0xEA, 0xEA,
		0x9A, 0x88, 0x01, 0x96, 0xBF, 0x79, 0x8C, 0xE1, 0xA8, 0x33,
	};

	static const Ssl::Fingerprint::Cert::Sha256 certSha256Fingerprint PROGMEM = {
		0xEC, 0xB2, 0x21, 0x7E, 0x43, 0xCC, 0x83, 0xE5, 0x5B, 0x35, 0x7F, 0x1A, 0xC2, 0x06, 0xE8, 0xBF,
		0xB1, 0x5F, 0x5B, 0xC8, 0x13, 0x9F, 0x93, 0x37, 0x3C, 0xF4, 0x8E, 0x82, 0xEC, 0x81, 0x28, 0xDF,
	};

	static const Ssl::Fingerprint::Pki::Sha256 publicKeyFingerprint PROGMEM = {
		0xad, 0xcc, 0x21, 0x92, 0x8e, 0x65, 0xc7, 0x54, 0xac, 0xac, 0xb8, 0x2f, 0x12, 0x95, 0x2e, 0x19,
		0x7d, 0x15, 0x7e, 0x32, 0xbe, 0x90, 0x27, 0x43, 0xab, 0xfe, 0xf1, 0xf2, 0xf2, 0xe1, 0x9c, 0x35,
	};

	// Trust certificate only if it matches the SHA1 fingerprint...
	session.validators.pin(sha1Fingerprint);

	// ... or if the public key matches the SHA256 fingerprint.
	session.validators.pin(publicKeyFingerprint);

	// We're using fingerprints, so don't attempt to validate full certificate
	session.options.verifyLater = true;

	// Go with maximum buffer sizes
	session.maxBufferSize = Ssl::MaxBufferSize::K16;

	// Use all supported cipher suites to make a connection
	session.cipherSuites = &Ssl::CipherSuites::full;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	connectTimer.start();

	auto request = new HttpRequest(F("https://www.grc.com/fingerprints.htm"));
	request->onSslInit(grcSslInit);
	request->onRequestComplete(onDownload);
	request->setResponseStream(new CounterStream);
	downloadClient.send(request);
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	Serial.println(F("I'm NOT CONNECTED!"));
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial
	Serial.println(F("Ready for SSL tests"));

#ifdef ENABLE_MALLOC_COUNT
	MallocCount::enableLogging(true);
#endif

	auto heapTimer = new Timer;
	heapTimer->initializeMs<5000>(printHeap).start();

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
