#include <SmingCore.h>
#include <Ota/Network/HttpUpgrader.h>
#include <Storage/PartitionStream.h>
#include <Storage/SpiFlash.h>
#include <Ota/Upgrader.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

Ota::Network::HttpUpgrader* otaUpdater;
Storage::Partition spiffsPartition;
OtaUpgrader ota;

Storage::Partition findSpiffsPartition(Storage::Partition appPart)
{
	String name = F("spiffs");
	name += ota.getSlot(appPart);
	auto part = Storage::findPartition(name);
	if(!part) {
		debug_w("Partition '%s' not found", name.c_str());
	}
	return part;
}

void upgradeCallback(Ota::Network::HttpUpgrader& client, bool result)
{
	Serial.println("In callback...");
	if(result == true) {
		// success
		ota.end();

		auto part = ota.getNextBootPartition();
		// set to boot new rom and then reboot
		Serial.printf(_F("Firmware updated, rebooting to %s @ ...\r\n"), part.name().c_str());
		ota.setBootPartition(part);
		System.restart();
	} else {
		ota.abort();
		// fail
		Serial.println(_F("Firmware update failed!"));
	}
}

void doUpgrade()
{
	Serial.println(F("Updating..."));

	// need a clean object, otherwise if run before and failed will not run again
	if(otaUpdater) {
		delete otaUpdater;
	}
	otaUpdater = new Ota::Network::HttpUpgrader();

	// select rom slot to flash
	auto part = ota.getNextBootPartition();

#ifndef RBOOT_TWO_ROMS
	// flash rom to position indicated in the rBoot config rom table
	otaUpdater->addItem(ROM_0_URL, part);
#else
	// flash appropriate ROM
	otaUpdater->addItem((ota.getSlot(part) == 0) ? ROM_0_URL : ROM_1_URL, part);
#endif

	ota.begin(part);

	auto spiffsPart = findSpiffsPartition(part);
	if(spiffsPart) {
		// use user supplied values (defaults for 4mb flash in hardware config)
		otaUpdater->addItem(SPIFFS_URL, spiffsPart, new Storage::PartitionStream(spiffsPart));
	}

	// request switch and reboot on success
	//otaUpdater->switchToRom(slot);
	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(upgradeCallback);

	// start update
	otaUpdater->start();
}

void doSwitch()
{
	auto before = ota.getRunningPartition();
	auto after = ota.getNextBootPartition();

	Serial.printf(_F("Swapping from %s @ 0x%08x to %s @ 0x%08x.\r\n"), before.name().c_str(), before.address(),
				  after.name().c_str(), after.address());
	if(ota.setBootPartition(after)) {
		Serial.println(F("Restarting...\r\n"));
		System.restart();
	} else {
		Serial.println(F("Switch failed."));
	}
}

void showInfo()
{
	Serial.printf(_F("\r\nSDK: v%s\r\n"), system_get_sdk_version());
	Serial.printf(_F("Free Heap: %lu\r\n"), system_get_free_heap_size());
	Serial.printf(_F("CPU Frequency: %lu MHz\r\n"), system_get_cpu_freq());
	Serial.printf(_F("System Chip ID: %lx\r\n"), system_get_chip_id());
	Serial.printf(_F("SPI Flash ID: %lx\r\n"), Storage::spiFlash->getId());
	Serial.printf(_F("SPI Flash Size: %ux\r\n"), Storage::spiFlash->getSize());

	auto before = ota.getRunningPartition();
	auto after = ota.getNextBootPartition();

	Serial.printf(_F("Current %s @ 0x%08lx, future %s @ 0x%0l8x\r\n"), before.name().c_str(), before.address(),
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
			Serial.print(" mac: ");
			Serial.println(WifiStation.getMacAddress());
		} else if(!strcmp(str, "ota")) {
			doUpgrade();
		} else if(!strcmp(str, "switch")) {
			doSwitch();
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
			Serial.printf(_F("filecount %u\r\n"), dir.count());
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
				Serial.println(F("Empty spiffs!"));
			}
		} else if(!strcmp(str, "info")) {
			showInfo();
		} else if(!strcmp(str, "help")) {
			Serial.println();
			Serial.println(F("available commands:"));
			Serial.println(F("  help - display this message"));
			Serial.println(F("  ip - show current ip address"));
			Serial.println(F("  connect - connect to wifi"));
			Serial.println(F("  restart - restart the device"));
			Serial.println(F("  switch - switch to the other rom and reboot"));
			Serial.println(F("  ota - perform ota update, switch rom and reboot"));
			Serial.println(F("  info - show device info"));
			if(spiffsPartition) {
				Serial.println(F("  ls - list files in spiffs"));
				Serial.println(F("  cat - show first file in spiffs"));
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
	spiffsPartition = findSpiffsPartition(partition);
	if(spiffsPartition) {
		debugf("trying to mount %s @ 0x%08x, length %d", spiffsPartition.name().c_str(), spiffsPartition.address(),
			   spiffsPartition.size());
		spiffs_mount(spiffsPartition);
	}

	WifiAccessPoint.enable(false);

	Serial.printf(_F("\r\nCurrently running %s @ 0x%08lx.\r\n"), partition.name().c_str(), partition.address());
	Serial.println(F("Type 'help' and press enter for instructions."));
	Serial.println();

	Serial.onDataReceived(serialCallBack);
}
