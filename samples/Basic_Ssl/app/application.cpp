#include <SmingCore.h>
#include "Data/HexString.h"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Timer procTimer;
HttpClient downloadClient;

/* Debug SSL functions */
void displaySessionId(SSL* ssl)
{
	const uint8_t* session_id = ssl_get_session_id(ssl);
	unsigned sess_id_size = ssl_get_session_id_size(ssl);

	if(sess_id_size != 0) {
		debugf("-----BEGIN SSL SESSION PARAMETERS-----");
		m_puts(makeHexString(session_id, sess_id_size).c_str());
		m_putc('\n');
		debugf("-----END SSL SESSION PARAMETERS-----");
	}
}

/**
 * Display what cipher we are using
 */
void displayCipher(SSL* ssl)
{
	m_printf("CIPHER is ");
	switch(ssl_get_cipher_id(ssl)) {
	case SSL_AES128_SHA:
		m_printf("AES128-SHA");
		break;

	case SSL_AES256_SHA:
		m_printf("AES256-SHA");
		break;

	case SSL_AES128_SHA256:
		m_printf("SSL_AES128_SHA256");
		break;

	case SSL_AES256_SHA256:
		m_printf("SSL_AES256_SHA256");
		break;

	default:
		m_printf("Unknown - %u", ssl_get_cipher_id(ssl));
		break;
	}

	m_printf("\n");
}

int onDownload(HttpConnection& connection, bool success)
{
	debugf("Got response code: %d", connection.getResponse()->code);
	debugf("Success: %d", success);

	SSL* ssl = connection.getSsl();
	if(ssl) {
		const char* common_name = ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME);
		if(common_name) {
			debugf("Common Name:\t\t\t%s\n", common_name);
		}
		displayCipher(ssl);
		displaySessionId(ssl);
	}

	return 0; // return 0 on success in your callbacks
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	// Use the Gibson Research fingerprints web page as an example. Unlike Google, the fingerprints don't change!
	static const uint8_t grcSha1Fingerprint[] PROGMEM = {0x15, 0x9A, 0x76, 0xC5, 0xAE, 0xF4, 0x90, 0x15, 0x79, 0xE6,
														 0xA4, 0x99, 0x96, 0xC1, 0xD6, 0xA1, 0xD9, 0x3B, 0x07, 0x43};

	static const uint8_t grcPublicKeyFingerprint[] PROGMEM = {
		0xEB, 0xA0, 0xFE, 0x70, 0xFE, 0xCB, 0xF8, 0xA8, 0x7A, 0xB9, 0x1D, 0xAC, 0x1E, 0xAC, 0xA0, 0xF6,
		0x62, 0xCB, 0xCD, 0xE4, 0x16, 0x72, 0xE6, 0xBC, 0x82, 0x9B, 0x32, 0x39, 0x43, 0x15, 0x76, 0xD4};

	debugf("Connected. Got IP: %s", ip.toString().c_str());

	HttpRequest* request = new HttpRequest(F("https://www.grc.com/fingerprints.htm"));
	request->setSslOptions(SSL_SERVER_VERIFY_LATER);

	/*
	 * GET probably won't work as sites tend to use 16K blocks which we can't handle,
	 * so just fetch the header and leave it at that. To return actual data requires a web server
	 * configured to use smaller encrytion blocks, e.g. 4K.
	 */
	request->setMethod(HTTP_HEAD);

	SslFingerprints fingerprints;

	/*
	 * The line below shows how to trust only a certificate that matches the SHA1 fingerprint.
	 */
	fingerprints.setSha1_P(grcSha1Fingerprint, sizeof(grcSha1Fingerprint));

	/*
	* The line below shows how to trust only a certificate in which the public key matches the SHA256 fingerprint.
	*/
	fingerprints.setSha256_P(grcPublicKeyFingerprint, sizeof(grcPublicKeyFingerprint));

	request->pinCertificate(fingerprints);
	request->onRequestComplete(onDownload);

	request->setResponseStream(&Serial);
	downloadClient.send(request);
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
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
