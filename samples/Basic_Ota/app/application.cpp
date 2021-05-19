#include <SmingCore.h>
#include <Network/RbootHttpUpdater.h>
#include <Storage/SpiFlash.h>
#include <Ota/Upgrader.h>

// download urls, set appropriately
#define ROM_0_URL "http://192.168.7.5:80/rom0.bin"
#define ROM_1_URL "http://192.168.7.5:80/rom1.bin"
#define SPIFFS_URL "http://192.168.7.5:80/spiff_rom.bin"

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

RbootHttpUpdater* otaUpdater;
Storage::Partition spiffsPartition;
OtaUpgrader ota;

Storage::Partition findSpiffsPartition(uint8_t slot)
{
	String name = F("spiffs");
	name += slot;
	auto part = Storage::findPartition(name);
	if(!part) {
		debug_w("Partition '%s' not found", name.c_str());
	}
	return part;
}

void otaUpdateCallBack(RbootHttpUpdater& client, bool result)
{
	Serial.println("In callback...");
	if(result == true) {
		// success
		ota.end();

		auto part = ota.getNextUpdatePartition();
		// set to boot new rom and then reboot
		Serial.printf("Firmware updated, rebooting to rom %s...\r\n", part.name().c_str());
		ota.setBootPartition(part);
		System.restart();
	} else {
		ota.abort();
		// fail
		Serial.println("Firmware update failed!");
	}
}

void OtaUpdate()
{
	Serial.println("Updating...");

	// need a clean object, otherwise if run before and failed will not run again
	if(otaUpdater) {
		delete otaUpdater;
	}
	otaUpdater = new RbootHttpUpdater();

	// select rom slot to flash
	auto part = ota.getNextUpdatePartition();

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(part.address(), ROM_0_URL, part.size());
#else
	// flash appropriate ROM
	otaUpdater->addItem(part.address(), (part.subType() == 0) ? ROM_0_URL : ROM_1_URL, part.size());
#endif

	ota.begin(part);

	auto spiffsPart = findSpiffsPartition(part.subType());
	if(spiffsPart) {
		// use user supplied values (defaults for 4mb flash in hardware config)
		otaUpdater->addItem(spiffsPart.address(), SPIFFS_URL, spiffsPart.size());
	}

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(otaUpdateCallBack);

	// start update
	otaUpdater->start();
}

void Switch()
{
	auto before = ota.getRunningPartition();
	auto after = ota.getNextUpdatePartition();

	Serial.printf(_F("Swapping from rom %s to rom %s.\r\n"), before.name().c_str(), after.name().c_str());
	if(ota.setBootPartition(after)) {
		Serial.println(F("Restarting...\r\n"));
		System.restart();
	} else {
		Serial.println(F("Switch failed."));
	}
}

void ShowInfo()
{
	Serial.printf("\r\nSDK: v%s\r\n", system_get_sdk_version());
	Serial.printf("Free Heap: %d\r\n", system_get_free_heap_size());
	Serial.printf("CPU Frequency: %d MHz\r\n", system_get_cpu_freq());
	Serial.printf("System Chip ID: %x\r\n", system_get_chip_id());
	Serial.printf("SPI Flash ID: %x\r\n", Storage::spiFlash->getId());
	Serial.printf("SPI Flash Size: %x\r\n", Storage::spiFlash->getSize());

	auto before = ota.getRunningPartition();
	auto after = ota.getNextUpdatePartition();

	Serial.printf(_F("Current rom: %s@%x, future rom: %s@%x\r\n"), before.name().c_str(), before.address(),
				  after.name().c_str(), after.address());
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
{
	int pos = stream.indexOf('\n');
	if(pos > -1) {
		char str[pos + 1];
		for(int i = 0; i < pos + 1; i++) {
			str[i] = stream.read();
			if(str[i] == '\r' || str[i] == '\n') {
				str[i] = '\0';
			}
		}

		if(!strcmp(str, "connect")) {
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
			WifiStation.connect();
		} else if(!strcmp(str, "ip")) {
			Serial.print("ip: ");
			Serial.print(WifiStation.getIP());
			Serial.print("mac: ");
			Serial.println(WifiStation.getMacAddress());
		} else if(!strcmp(str, "ota")) {
			OtaUpdate();
		} else if(!strcmp(str, "switch")) {
			Switch();
		} else if(!strcmp(str, "restart")) {
			System.restart();
		} else if(!strcmp(str, "ls")) {
			Directory dir;
			if(dir.open()) {
				while(dir.next()) {
					Serial.print("  ");
					Serial.println(dir.stat().name);
				}
			}
			Serial.printf("filecount %d\r\n", dir.count());
		} else if(!strcmp(str, "cat")) {
			Directory dir;
			if(dir.open() && dir.next()) {
				auto filename = dir.stat().name.c_str();
				Serial.printf("dumping file %s:\r\n", filename);
				// We don't know how big the is, so streaming it is safest
				FileStream fs;
				fs.open(filename);
				Serial.copyFrom(&fs);
				Serial.println();
			} else {
				Serial.println("Empty spiffs!");
			}
		} else if(!strcmp(str, "info")) {
			ShowInfo();
		} else if(!strcmp(str, "help")) {
			Serial.println();
			Serial.println("available commands:");
			Serial.println("  help - display this message");
			Serial.println("  ip - show current ip address");
			Serial.println("  connect - connect to wifi");
			Serial.println("  restart - restart the esp8266");
			Serial.println("  switch - switch to the other rom and reboot");
			Serial.println("  ota - perform ota update, switch rom and reboot");
			Serial.println("  info - show esp8266 info");
			if(spiffsPartition) {
				Serial.println("  ls - list files in spiffs");
				Serial.println("  cat - show first file in spiffs");
			}
			Serial.println();
		} else {
			Serial.println("unknown command");
		}
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default
	Serial.systemDebugOutput(true); // Debug output to serial

	// mount spiffs
	auto partition = ota.getRunningPartition();
	spiffsPartition = findSpiffsPartition(partition.subType());
	if(spiffsPartition) {
		debugf("trying to mount '%s' at 0x%08x, length %d", spiffsPartition.name().c_str(), spiffsPartition.address(),
			   spiffsPartition.size());
		spiffs_mount(spiffsPartition);
	}

	WifiAccessPoint.enable(false);

	Serial.printf("\r\nCurrently running rom %s.\r\n", partition.name().c_str());
	Serial.println("Type 'help' and press enter for instructions.");
	Serial.println();

	Serial.onDataReceived(serialCallBack);
}
