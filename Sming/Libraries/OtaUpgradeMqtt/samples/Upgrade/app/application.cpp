#include <SmingCore.h>
#include <Storage/SpiFlash.h>
#include <Ota/Manager.h>
#include <OtaUpgrade/Mqtt/StandardPayloadParser.h>

#if ENABLE_OTA_ADVANCED
#include <OtaUpgrade/Mqtt/AdvancedPayloadParser.h>
#endif

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

#ifndef APP_VERSION
#include <SmingVersion.h>
#define APP_VERSION MACROQUOTE(SMING_MAJOR_VERSION) "." MACROQUOTE(SMING_MINOR_VERSION)
#define APP_VERSION_PATCH SMING_PATCH_VERSION
#endif

namespace
{
MqttClient mqtt;

#if ENABLE_CLIENT_CERTIFICATE
IMPORT_FSTR(privateKeyData, PROJECT_DIR "/files/private.pem.key.der");
IMPORT_FSTR(certificateData, PROJECT_DIR "/files/certificate.pem.crt.der");
#endif

void otaUpdate()
{
	if(mqtt.isProcessing()) {
		Serial.println("There is an update in progress. Refusing to start new update.");
		return;
	}

	Serial.println("Checking for a new application firmware...");

	auto part = OtaManager.getBootPartition();
	if(!part) {
		Serial.println("FAILED: Cannot find application address");
		return;
	}

#ifdef ENABLE_SSL
	mqtt.setSslInitHandler([](Ssl::Session& session) {
		// These fingerprints change very frequently.
		static const Ssl::Fingerprint::Cert::Sha1 sha1Fingerprint PROGMEM = {MQTT_FINGERPRINT_SHA1};

		// Trust certificate only if it matches the SHA1 fingerprint...
		session.validators.pin(sha1Fingerprint);

		// We're using fingerprints, so don't attempt to validate full certificate
		session.options.verifyLater = true;

#if ENABLE_CLIENT_CERTIFICATE
		session.keyCert.assign(privateKeyData, certificateData);
#endif

		// Use all supported cipher suites to make a connection
		session.cipherSuites = &Ssl::CipherSuites::full;
	});
#endif

	mqtt.connect(Url(MQTT_URL), "sming");

#if ENABLE_OTA_ADVANCED
	/*
	 * The advanced parser suppors all firmware upgrades supported by the `OtaUpgrade` library.
	 * `OtaUpgrade` library provides firmware signing, firmware encryption and so on.
	 */
	auto parser = new OtaUpgrade::Mqtt::AdvancedPayloadParser(APP_VERSION_PATCH);
#else
	/*
	 * The command below uses class that stores the firmware directly
	 * using RbootOutputStream on a location provided by us
	 */
	auto parser = new OtaUpgrade::Mqtt::StandardPayloadParser(part, APP_VERSION_PATCH);
#endif

	mqtt.setPayloadParser([parser](MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer,
								   int length) -> int { return parser->parse(state, message, buffer, length); });

	String updateTopic = "a/";
	updateTopic += APP_ID;
	updateTopic += "/u/";
	updateTopic += APP_VERSION;
	debug_d("Subscribing to topic: %s", updateTopic.c_str());
	mqtt.subscribe(updateTopic);
}

void showInfo()
{
	Serial.printf(_F("\r\nSDK: v%s\r\n"), system_get_sdk_version());
	Serial.printf(_F("Free Heap: %d\r\n"), system_get_free_heap_size());
	Serial.printf(_F("CPU Frequency: %d MHz\r\n"), system_get_cpu_freq());
	Serial.printf(_F("System Chip ID: %x\r\n"), system_get_chip_id());

	int total = 0;
	for(auto it = OtaManager.getBootPartitions(); it; ++it) {
		auto part = *it;
		debug_d("ROM %s: 0x%08x, SubType: %s", part.name(), part.address(),
				toLongString(part.type(), part.subType()).c_str());
		total++;
	}
	debug_d("=======================");
	debug_d("Bootable ROMs found: %d", total);

	auto part = OtaManager.getRunningPartition();

	Serial.printf(_F("\r\nCurrently running %s: 0x%08x. Application version: %s\r\n"), part.name(), part.address(),
				  APP_VERSION);
	Serial.println();
}

void connectOk(IpAddress ip, IpAddress mask, IpAddress gateway)
{
	/*
	 This application starts the update right after a successful connecton.
	 In a real-world application you should start the update procedure
	 only when the chances of success are high enough.

	 For example when there is enough power, free RAM and sufficient time
	 AND there is no critical task running at the moment.
	*/
	otaUpdate();
}

} // end of anonymous namespace

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	showInfo();

	WifiStation.enable(true);
	WifiStation.config(WIFI_SSID, WIFI_PWD);

	WifiEvents.onStationGotIP(connectOk);
}
