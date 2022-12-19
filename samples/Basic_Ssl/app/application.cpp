#include <SmingCore.h>
#include <CounterStream.h>

#ifdef ENABLE_MALLOC_COUNT
#include <malloc_count.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;
HttpClient downloadClient;
OneShotFastMs connectTimer;

void printHeap()
{
	Serial.println(_F("Heap statistics"));
	Serial << _F("  Free bytes:  ") << system_get_free_heap_size() << endl;
#ifdef ENABLE_MALLOC_COUNT
	Serial << _F("  Used:        ") << MallocCount::getCurrent() << endl;
	Serial << _F("  Peak used:   ") << MallocCount::getPeak() << endl;
	Serial << _F("  Allocations: ") << MallocCount::getAllocCount() << endl;
	Serial << _F("  Total used:  ") << MallocCount::getTotal() << endl;
#endif
}

int onDownload(HttpConnection& connection, bool success)
{
	auto elapsed = connectTimer.elapsedTime();

	auto status = connection.getResponse()->code;
	Serial << _F("Got response code: ") << unsigned(status) << " (" << status << _F("), success: ") << success;

	auto stream = connection.getResponse()->stream;
	assert(stream != nullptr);

	Serial << _F(", received ") << stream->available() << _F(" bytes") << endl;

	auto& headers = connection.getResponse()->headers;
	for(unsigned i = 0; i < headers.count(); ++i) {
		Serial.print(headers[i]);
	}

	auto ssl = connection.getSsl();
	if(ssl != nullptr) {
		Serial.print(*ssl);
	}

	Serial << _F("Time to connect and download page: ") << elapsed.toString() << endl;

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
		0xC3, 0xFB, 0x91, 0x85, 0xCC, 0x6B, 0x4C, 0x7D, 0xE7, 0x18,
		0xED, 0xD8, 0x00, 0xD2, 0x84, 0xE7, 0x6E, 0x97, 0x06, 0x07,
	};

	static const Ssl::Fingerprint::Cert::Sha256 certSha256Fingerprint PROGMEM = {
		0xC3, 0xFB, 0x91, 0x85, 0xCC, 0x6B, 0x4C, 0x7D, 0xE7, 0x18,
		0xED, 0xD8, 0x00, 0xD2, 0x84, 0xE7, 0x6E, 0x97, 0x06, 0x07,
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
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put your SSID and password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
