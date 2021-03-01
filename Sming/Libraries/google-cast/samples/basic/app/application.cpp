#include <SmingCore.h>
#include <Network/HttpClient.h>
#include <Network/GoogleCast/Client.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

GoogleCast::Client castClient;

bool onMessage(GoogleCast::ChannelMessage& message)
{
	if((message.payloadType() == message.PayloadType::string)) {
		auto value = message.payload_utf8.getData();
		auto len = message.payload_utf8.getLength();

		m_puts("Response: ");
		m_nputs(reinterpret_cast<const char*>(value), len);
		m_puts("\r\n");

		debug_i("source_id: %s", String(message.source_id).c_str());
		debug_i("destination_id: %s", String(message.destination_id).c_str());
		debug_i("nameSpace: %s", String(message.nameSpace).c_str());

		// DynamicJsonDocument doc(1024);
		// Json::deserialize(doc, value, len);
	} else {
		m_printHex("RESPONSE", message.payload_binary.getData(), message.payload_binary.getLength());
	}

	return true;
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(F("Connected. Got IP: "));
	Serial.println(ip);

	Serial.println(F("Connecting to your Smart TV"));
	castClient.connect(IpAddress("192.168.1.103"));

	Serial.println(F("Starting YouTube"));
	castClient.launch("YouTube");
	castClient.setResponseHandler(onMessage);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(connectOk);
}
