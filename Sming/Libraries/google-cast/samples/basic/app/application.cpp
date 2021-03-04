#include <SmingCore.h>
#include <Network/GoogleCast/Client.h>
#include <Network/GoogleCast/ClientInfo.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and Password here
#define WIFI_PWD "PleaseEnterPass"
#endif

GoogleCast::ClientInfo clientInfo;
GoogleCast::Client castClient;
Timer statusTimer;

static constexpr float skipSecs{10};

struct Session {
	enum class State {
		idle,
		playRequestSent,
		loading,
		playing,
		paused,
		stopped,
	};

	int id;
	State state;
	double currentTime;

	void playPause()
	{
		if(state == State::playing) {
			castClient.media.pause(id);
			state = State::paused;
		} else if(state == State::paused) {
			castClient.media.play(id);
			state = State::playing;
		}
	}

	void seekRelative(float secs)
	{
		castClient.media.seek(id, currentTime + secs);
	}

	void updateState(const char* playerState)
	{
		if(F("LOADING") == playerState) {
			state = State::loading;
		} else if(F("PLAYING") == playerState) {
			state = State::playing;
		} else if(F("PAUSED") == playerState) {
			state = State::paused;
		} else {
			return;
		}
		debug_i("** State changed ** ");
	}
};
Session activeSession;

static constexpr unsigned minStatusInterval{10000};

DEFINE_FSTR(APPID_BACKDROP, "E8C28D3C") // Idle screen
DEFINE_FSTR(APPID_MEDIA_RECEIVER, "CC1AD845")
DEFINE_FSTR(APPID_CHROMECAST, "ChromeCast")
DEFINE_FSTR(APPID_YOUTUBE, "YouTube")
DEFINE_FSTR(APPID_SCREEN_MIRRORING, "674A0243")
DEFINE_FSTR(APPID_BUBBLE_UPNP, "3927FA74")
DEFINE_FSTR(APPID_YOUTUBE_MUSIC, "2DB7CC49")
DEFINE_FSTR(APPID_GOOGLE_PHOTOS, "5FD0CDC9")
DEFINE_FSTR(APPID_MUSIC, "F3F3F51B")
DEFINE_FSTR(APPID_CHROME_MIRRORING, "0F5096E8")
DEFINE_FSTR(APPID_GOOGLE_PLAY_MOVIES, "9381F2BD")
DEFINE_FSTR(APPID_BBC_IPLAYER, "19A3DCE0")
DEFINE_FSTR(APPID_YOUTUBE_TV, "4475D545")
DEFINE_FSTR(APPID_DEEZER, "28BE5D9A")
DEFINE_FSTR(APPID_TWITCH, "358E83DC")
DEFINE_FSTR(APPID_GOOGLE_MUSIC, "GoogleMusic")
DEFINE_FSTR(APPID_PLEX, "9AC194DC")
DEFINE_FSTR(APPID_GOOGLE_PLUS, "1A27E40D")
DEFINE_FSTR(APPID_NETFLIX, "CA5E8412")

DEFINE_FSTR(mediaUrl, "http://commondatastorage.googleapis.com/gtv-videos-bucket/sample/BigBuckBunny.mp4");
DEFINE_FSTR(mediaMimeType, "video/mp4")

// Pretty-print a JSON document
void printJson(JsonDocument& doc)
{
	Serial.print(F("Message: "));
	Json::serialize(doc, Serial, Json::SerializationFormat::Pretty);
	Serial.println();
}

bool onReceiverMessage(GoogleCast::Channel& channel, GoogleCast::Channel::Message& message)
{
	debug_i("%s", __PRETTY_FUNCTION__);
	statusTimer.restart();

	DynamicJsonDocument doc(1024);
	if(!message.deserialize(doc)) {
		return true;
	}

	printJson(doc);
	String type = doc["type"];
	if(type != "RECEIVER_STATUS") {
		return true;
	}

	auto status = doc["status"];
	auto app = status["applications"][0];
	if(!app || APPID_MEDIA_RECEIVER != app["appId"]) {
		return true;
	}

	if(activeSession.state != Session::State::idle) {
		return true;
	}

	String transportId = app["transportId"].as<const char*>();

	// Connect to the application
	castClient.connection.setDestinationId(transportId);
	castClient.connection.connect();

	castClient.media.setDestinationId(transportId);
	if(castClient.media.load(mediaUrl, mediaMimeType)) {
		activeSession.state = Session::State::playRequestSent;
	}

	return true;
}

bool onMediaMessage(GoogleCast::Channel& channel, GoogleCast::Channel::Message& message)
{
	debug_i("%s", __PRETTY_FUNCTION__);

	DynamicJsonDocument doc(1024);
	if(!message.deserialize(doc)) {
		return true;
	}

	printJson(doc);
	String type = doc["type"];
	if(type != "MEDIA_STATUS") {
		return true;
	}

	auto status = doc["status"][0];
	activeSession.updateState(status["playerState"].as<const char*>());
	activeSession.id = status["mediaSessionId"];
	activeSession.currentTime = status["currentTime"];

	return true;
}

bool onMessage(GoogleCast::Channel::Message& message)
{
	debug_i("%s", __PRETTY_FUNCTION__);
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

void selectDevice()
{
	if(clientInfo.count() == 0) {
		Serial.println(F("No devices found"));
		return;
	}

	for(unsigned i = 0; i < clientInfo.count(); ++i) {
		Serial.print(i + 1);
		Serial.print(". ");
		Serial.print(clientInfo.valueAt(i));
		Serial.print(" - ");
		Serial.println(clientInfo.keyAt(i));
	}

	Serial.print(F("Enter device number: "));
}

void connectDevice(unsigned index)
{
	if(index >= clientInfo.count()) {
		selectDevice();
		return;
	}

	auto friendlyName = clientInfo.keyAt(index);

	Serial.print(F("Connecting to '"));
	Serial.print(friendlyName);
	Serial.println("'...");

	castClient.onConnect([](bool success) {
		Serial.print(F("Client connect: "));
		Serial.println(success ? "OK" : "FAIL");
		if(!success) {
			return;
		}

		castClient.receiver.launch(APPID_MEDIA_RECEIVER);

		// Serial.println(F("Starting YouTube"));
		// castClient.receiver.launch("YouTube");
		statusTimer.initializeMs<minStatusInterval>(InterruptCallback([]() { castClient.receiver.getStatus(); }))
			.start();
	});
	castClient.receiver.onMessage(onReceiverMessage);
	castClient.media.onMessage(onMediaMessage);
	castClient.onMessage(onMessage);

	statusTimer.initializeMs<1000>([index]() {
		auto deviceAddress = clientInfo.valueAt(index);
		castClient.connect(deviceAddress);
	});
	statusTimer.startOnce();
}

void discoverDevices()
{
	if(clientInfo.startDiscovery()) {
		Serial.println(F("Discovering devices..."));
	} else {
		Serial.println(F("ERROR! Device discovery cannot be started"));
		return;
	}

	statusTimer.initializeMs<3000>(selectDevice).startOnce();
}

void onGotIp(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	Serial.print(F("Got IP: "));
	Serial.println(ip);

	discoverDevices();
}

void onSerialData(Stream& source, char arrivedChar, uint16_t availableCharsCount)
{
	// Take only first character, discard the rest
	while(availableCharsCount--) {
		source.read();
	}
	switch(arrivedChar) {
	case ' ':
		activeSession.playPause();
		break;

	case 'd':
	case 'D':
		discoverDevices();
		break;

	case 'f':
	case 'F':
		activeSession.seekRelative(skipSecs);
		break;

	case 'r':
	case 'R':
		activeSession.seekRelative(-skipSecs);
		break;

	case 'q':
	case 'Q':
		// castClient.receiver.launch(APPID_BACKDROP);
		castClient.connection.close();
		break;

	default:
		connectDevice(arrivedChar - '1');
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	Serial.onDataReceived(onSerialData);

	// Setup the WIFI connection
	WifiStation.enable(true);
	WifiAccessPoint.enable(false);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(onGotIp);
}
