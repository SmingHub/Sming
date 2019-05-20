/**
 * Please, note, that in order to run this sample you should recompile Sming with ENABLE_SSL=1.
 * The following three commands should be enough:
 *
 * cd Sming/Sming
 * make clean
 * make ENABLE_SSL=1
 *
 *
 * This example talks to SSL enabled TCP server. If you do not have such you can use ncat (from the nmap package) and run something like this
 *
 * ncat -vvv  -l 0.0.0.0 4444 --ssl --ssl-key ~/Private/x.key --ssl-cert ~/Private/x.cert
 *
 */

#include <SmingCore.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put you SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#ifndef SERVER_IP
// Make sure to define the server IP in the code"
#define SERVER_IP "127.0.0.1"
#endif

Timer procTimer;
TcpClient* client;

bool showMeta = true;

/* Debug SSL functions */
void displaySessionId(SSL* ssl)
{
	int i;
	const uint8_t* session_id = ssl_get_session_id(ssl);
	int sess_id_size = ssl_get_session_id_size(ssl);

	if(sess_id_size > 0) {
		debugf("-----BEGIN SSL SESSION PARAMETERS-----\n");
		for(i = 0; i < sess_id_size; i++) {
			debugf("%02x", session_id[i]);
		}

		debugf("\n-----END SSL SESSION PARAMETERS-----\n");
	}
}

/**
 * Display what cipher we are using
 */
void displayCipher(SSL* ssl)
{
	Serial.printf("CIPHER is ");
	switch(ssl_get_cipher_id(ssl)) {
	case SSL_AES128_SHA:
		Serial.printf("AES128-SHA");
		break;

	case SSL_AES256_SHA:
		Serial.printf("AES256-SHA");
		break;

	case SSL_AES128_SHA256:
		Serial.printf("SSL_AES128_SHA256");
		break;

	case SSL_AES256_SHA256:
		Serial.printf("SSL_AES256_SHA256");
		break;

	default:
		Serial.printf("Unknown - %d", ssl_get_cipher_id(ssl));
		break;
	}

	Serial.printf("\n");
}

bool onReceive(TcpClient& tcpClient, char* data, int size)
{
	debugf("Got data with size: %d", size);
	debugf("Free heap: %d", system_get_free_heap_size());
	if(size < 1) {
		return false;
	}

	if(showMeta) {
		SSL* ssl = tcpClient.getSsl();
		if(ssl) {
			const char* common_name = ssl_get_cert_dn(ssl, SSL_X509_CERT_COMMON_NAME);
			if(common_name) {
				debugf("Common Name:\t\t\t%s\n", common_name);
			}
			displayCipher(ssl);
			displaySessionId(ssl);
		}
		debugf("end of meta...");
		showMeta = false;
	}

	debugf("Ready to send back the data...");

	return tcpClient.send(data, size);
}

void connectFail(String ssid, uint8_t ssid_len, uint8_t bssid[6], uint8_t reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	debugf("IP: %s", ip.toString().c_str());
	client = new TcpClient(TcpClientDataDelegate(onReceive));
	client->addSslOptions(SSL_SERVER_VERIFY_LATER);
	client->connect(IPAddress(SERVER_IP), 4444, true);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true); // Allow debug print to serial

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD); // Put you SSID and Password here

	// Run our method when station was connected to AP (or not connected)
	WifiEvents.onStationDisconnect(connectFail);
	WifiEvents.onStationGotIP(gotIP);
}
