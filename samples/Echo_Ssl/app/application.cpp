/**
 *
 * This example talks to SSL enabled TCP server. If you do not have such you can use ncat (from the nmap package) and run something like this
 *
 * ncat -vvv  -l 0.0.0.0 4444 --ssl --ssl-key ~/Private/x.key --ssl-cert ~/Private/x.cert
 *
 */

#include <SmingCore.h>
#include <Data/HexString.h>

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
void displaySessionId(SslSessionId* sessionId)
{
	if(sessionId != nullptr && sessionId->isValid()) {
		debugf("-----BEGIN SSL SESSION PARAMETERS-----");
		debugf("%s", makeHexString(sessionId->getValue(), sessionId->getLength()).c_str());
		debugf("-----END SSL SESSION PARAMETERS-----");
	}
}

bool onReceive(TcpClient& tcpClient, char* data, int size)
{
	debugf("Got data with size: %d", size);
	debugf("Free heap: %d", system_get_free_heap_size());
	if(size < 1) {
		return false;
	}

	if(showMeta) {
		SslConnection* ssl = tcpClient.getSsl();
		if(ssl) {
			SslCertificate* cert = ssl->getCertificate();
			debugf("Common Name:\t\t\t%s\n", cert->getName(eSCN_CERT_COMMON_NAME).c_str());
			debugf("Cipher: %s", ssl->getCipher().c_str());
			displaySessionId(ssl->getSessionId());
		}
		debugf("end of meta...");
		showMeta = false;
	}

	debugf("Ready to send back the data...");

	return tcpClient.send(data, size);
}

void connectFail(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)
{
	debugf("I'm NOT CONNECTED!");
}

void gotIP(IpAddress ip, IpAddress netmask, IpAddress gateway)
{
	debugf("IP: %s", ip.toString().c_str());
	client = new TcpClient(TcpClientDataDelegate(onReceive));
	client->addSslOptions(SSL_SERVER_VERIFY_LATER);
	client->connect(IpAddress(SERVER_IP), 4444, true);
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
