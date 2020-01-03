#include <SmingCore.h>
#include <CounterStream.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;
HttpClient downloadClient;

int onDownload(HttpConnection& connection, bool success)
{
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

	return 0; // return 0 on success in your callbacks
}

/*
 * Initialise SSL session parameters for connecting to the GRC web server
 */
void grcSslInit(Ssl::Session& session, HttpRequest& request)
{
	debug_i("Initialising SSL session for GRC");

	// Use the Gibson Research fingerprints web page as an example. Unlike Google, the fingerprints don't change!
	static const Ssl::Fingerprint::Cert::Sha1 sha1Fingerprint PROGMEM = {
		0x15, 0x9A, 0x76, 0xC5, 0xAE, 0xF4, 0x90, 0x15, 0x79, 0xE6,
		0xA4, 0x99, 0x96, 0xC1, 0xD6, 0xA1, 0xD9, 0x3B, 0x07, 0x43,
	};

	static const Ssl::Fingerprint::Cert::Sha256 certSha256Fingerprint PROGMEM = {
		0xEB, 0xA0, 0xFE, 0x70, 0xFE, 0xCB, 0xF8, 0xA8, 0x7A, 0xB9, 0x1D, 0xAC, 0x1E, 0xAC, 0xA0, 0xF6,
		0x62, 0xCB, 0xCD, 0xE4, 0x16, 0x72, 0xE6, 0xBC, 0x82, 0x9B, 0x32, 0x39, 0x43, 0x15, 0x76, 0xD4,
	};

	static const Ssl::Fingerprint::Pki::Sha256 publicKeyFingerprint PROGMEM = {
		0x3a, 0xb4, 0xa6, 0xf2, 0x36, 0xc1, 0x28, 0xc8, 0xac, 0x95, 0x12, 0xf3, 0x5e, 0xd8, 0x2d, 0xfc,
		0x33, 0x64, 0xc3, 0x79, 0x93, 0x8e, 0x7c, 0xbc, 0x6d, 0xfe, 0x97, 0xa0, 0x65, 0x7e, 0x9d, 0xa3};

	// Trust certificate only if it matches the SHA1 fingerprint...
	session.validators.pin(sha1Fingerprint);

	// ... or if the public key matches the SHA256 fingerprint.
	session.validators.pin(publicKeyFingerprint);

	// We're using fingerprints, so don't attempt to validate full certificate
	session.options.verifyLater = true;

	// Go with maximum buffer sizes
	session.maxBufferSize = Ssl::MaxBufferSize::K16;
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

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

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	WifiEvents.onStationGotIP(gotIP);
	WifiEvents.onStationDisconnect(connectFail);
}
