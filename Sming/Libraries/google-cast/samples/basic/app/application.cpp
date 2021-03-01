#include <SmingCore.h>
#include <Network/GoogleCast/Client.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

GoogleCast::Client castClient;
String activeSessionId;
Timer statusTimer;

static constexpr unsigned minStatusInterval{10000};

// Pretty-print a JSON document
void printJson(JsonDocument& doc)
{
	Serial.print(F("Message: "));
	Json::serialize(doc, Serial, Json::SerializationFormat::Pretty);
	Serial.println();
}

bool onReceiverMessage(GoogleCast::Channel& channel, GoogleCast::Channel::Message& message)
{
	statusTimer.restart();

	DynamicJsonDocument doc(1024);
	if(message.deserialize(doc)) {
		printJson(doc);
		String type = doc["type"];
		if(type == "RECEIVER_STATUS") {
			auto app = doc["status"]["applications"][0];
			if(app) {
				activeSessionId = app["sessionId"].as<const char*>();
				debug_i("Status: %s, session: %s", app["statusText"].as<const char*>(), activeSessionId.c_str());
			}
		}
	}

	return true;
}

bool onMessage(GoogleCast::Channel::Message& message)
{
	if((message.payloadType() == message.PayloadType::string)) {
		debug_i("nameSpace: %s, source: %s, destination: %s", String(message.nameSpace).c_str(),
				String(message.source_id).c_str(), String(message.destination_id).c_str());

		DynamicJsonDocument doc(1024);
		message.deserialize(doc);
		debug_i("doc memory usage: %u, data length: %u", doc.memoryUsage(), message.payload_utf8.getLength());

		printJson(doc);

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
	castClient.connect(IpAddress("192.168.1.118"));

	// Allow time to connect then launch an application
	auto timer = new AutoDeleteTimer;
	timer->initializeMs<1000>(InterruptCallback([]() {
		Serial.println(F("Starting YouTube"));
		castClient.receiver.launch("YouTube");
		castClient.receiver.getStatus();
	}));
	timer->startOnce();

	// Ignore heartbeat messages
	// castClient.heartbeat.onMessage(
	// 	[](GoogleCast::Channel& channel, GoogleCast::Channel::Message& message) { return true; });

	castClient.receiver.onMessage(onReceiverMessage);
	castClient.onMessage(onMessage);

	statusTimer.initializeMs<minStatusInterval>(InterruptCallback([]() { castClient.receiver.getStatus(); })).start();
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
