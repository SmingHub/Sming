#include <SmingCore.h>
#include <Data/Stream/RbootOutputStream.h>

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

struct UpdateState {
	RbootOutputStream* stream{nullptr};
	bool started{false};
};

Storage::Partition findRomPartition(uint8_t slot)
{
	String name = F("rom");
	name += slot;
	auto part = Storage::findPartition(name);
	if(!part) {
		debug_w("Partition '%s' not found", name.c_str());
	}
	return part;
}

void switchRom()
{
	uint8 before, after;
	before = rboot_get_current_rom();
	if(before == 0) {
		after = 1;
	} else {
		after = 0;
	}
	Serial.printf("Swapping from rom %d to rom %d.\r\n", before, after);
	rboot_set_current_rom(after);
	Serial.println("Restarting...\r\n");
	System.restart();
}

void otaUpdate()
{
	if(mqtt.isProcessing()) {
		Serial.println("There is an update in progress. Refusing to start new update.");
		return;
	}

	uint8 slot;
	rboot_config bootconf;
	// select rom slot to flash
	bootconf = rboot_get_config();
	slot = bootconf.current_rom;
	if(slot == 0) {
		slot = 1;
	} else {
		slot = 0;
	}

	Serial.println("Checking for a new application firmware...");

	auto part = findRomPartition(slot);
	if(!part) {
		Serial.println("FAILED: Cannot find application address");
		return;
	}

	mqtt.connect(Url(MQTT_URL), "sming");
	mqtt.setPayloadParser(
		[part](MqttPayloadParserState& state, mqtt_message_t* message, const char* buffer, int length) -> int {
			if(message == nullptr) {
				debug_e("Invalid MQTT message");
				return 1;
			}

			if(length == MQTT_PAYLOAD_PARSER_START) {
				UpdateState* updateState = new UpdateState();
				updateState->stream = nullptr;
				updateState->started = false;

				state.offset = 0;
				state.userData = updateState;
				return 0;
			}

			auto updateState = static_cast<UpdateState*>(state.userData);
			if(updateState == nullptr) {
				debug_e("Update failed for unknown reason!");
				return -1;
			}

			if(length == MQTT_PAYLOAD_PARSER_END) {
				bool skip = (updateState->stream == nullptr);
				if(!skip) {
					delete updateState->stream;
					switchRom();
				}

				return 0;
			}

			if(!updateState->started) {
				updateState->started = true;
				if(message->common.length - 1 > part.size()) {
					debug_e("The new rom is too big to fit!");
					return -2;
				}

				uint8_t patchVersion = buffer[0];
				if(patchVersion < APP_VERSION_PATCH) {
					// The update is not newer than our patch version
					return 0;
				}

				updateState->started = true;
				updateState->stream = new RbootOutputStream(part.address(), part.size());
				buffer++;
				length--;
			}

			auto rbootStream = static_cast<RbootOutputStream*>(updateState->stream);
			if(rbootStream == nullptr) {
				return 0;
			}

			auto written = rbootStream->write(reinterpret_cast<const uint8_t*>(buffer), length);
			return (written - length);
		});

	String updateTopic = "/a/";
	updateTopic += APP_ID;
	updateTopic += "/u/";
	updateTopic += APP_VERSION;
	debug_d("Subscribing to topic: %s", updateTopic.c_str());
	mqtt.subscribe(updateTopic);
}

void showInfo()
{
	Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
	Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
	Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
	Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());

	rboot_config conf;
	conf = rboot_get_config();

	debug_d("Count: %d", conf.count);
	debug_d("ROM 0: 0x%08x", conf.roms[0]);
	debug_d("ROM 1: 0x%08x", conf.roms[1]);
	debug_d("ROM 2: 0x%08x", conf.roms[2]);
	debug_d("GPIO ROM: %d", conf.gpio_rom);

	Serial.printf("\r\nCurrently running rom %d. Application version: %s\r\n", conf.current_rom, APP_VERSION);
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
