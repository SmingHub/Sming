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

std::unique_ptr<Ota::Network::HttpUpgrader> otaUpdater;
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
	Serial.println(_F("In callback..."));
	if(result == true) {
		// success
		ota.end();

		auto part = ota.getNextBootPartition();
		// set to boot new rom and then reboot
		Serial << _F("Firmware updated, rebooting to ") << part.name() << _F(" @ ...") << endl;
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
	otaUpdater.reset(new Ota::Network::HttpUpgrader);

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
		otaUpdater->addItem(SPIFFS_URL, spiffsPart, new Storage::PartitionStream(spiffsPart, true));
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

	Serial << _F("Swapping from ") << before.name() << " @ 0x" << String(before.address(), HEX) << " to "
		   << after.name() << " @ 0x" << String(after.address(), HEX) << endl;
	if(ota.setBootPartition(after)) {
		Serial.println(F("Restarting...\r\n"));
		System.restart();
	} else {
		Serial.println(F("Switch failed."));
	}
}

void showInfo()
{
	Serial.println();
	Serial << _F("SDK: v") << system_get_sdk_version() << endl;
	Serial << _F("Free Heap: ") << system_get_free_heap_size() << endl;
	Serial << _F("CPU Frequency: ") << system_get_cpu_freq() << " MHz" << endl;
	Serial << _F("System Chip ID: ") << String(system_get_chip_id(), HEX) << endl;
	Serial << _F("SPI Flash ID: ") << String(Storage::spiFlash->getId(), HEX) << endl;
	Serial << _F("SPI Flash Size: ") << String(Storage::spiFlash->getSize(), HEX) << endl;

	auto before = ota.getRunningPartition();
	auto after = ota.getNextBootPartition();

	Serial << _F("Current ") << before.name() << " @ 0x" << String(before.address(), HEX) << ", future " << after.name()
		   << " @ 0x" << String(after.address(), HEX) << endl;
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

		if(F("connect") == str) {
			// connect to wifi
			WifiStation.config(WIFI_SSID, WIFI_PWD);
			WifiStation.enable(true);
			WifiStation.connect();
		} else if(F("ip") == str) {
			Serial << "ip: " << WifiStation.getIP() << ", mac: " << WifiStation.getMacAddress() << endl;
		} else if(F("ota") == str) {
			doUpgrade();
		} else if(F("switch") == str) {
			doSwitch();
		} else if(F("restart") == str) {
			System.restart();
		} else if(F("ls") == str) {
			Directory dir;
			if(dir.open()) {
				while(dir.next()) {
					Serial << "  " << dir.stat().name << endl;
				}
			}
			Serial << _F("filecount ") << dir.count() << endl;
		} else if(F("cat") == str) {
			Directory dir;
			if(dir.open() && dir.next()) {
				auto filename = dir.stat().name.c_str();
				Serial << "dumping file " << filename << ": " << endl;
				// We don't know how big the is, so streaming it is safest
				FileStream fs;
				fs.open(filename);
				Serial.copyFrom(&fs);
				Serial.println();
			} else {
				Serial.println(F("Empty spiffs!"));
			}
		} else if(F("info") == str) {
			showInfo();
		} else if(F("help") == str) {
			Serial.print(_F("\r\n"
							"available commands:\r\n"
							"  help - display this message\r\n"
							"  ip - show current ip address\r\n"
							"  connect - connect to wifi\r\n"
							"  restart - restart the device\r\n"
							"  switch - switch to the other rom and reboot\r\n"
							"  ota - perform ota update, switch rom and reboot\r\n"
							"  info - show device info\r\n"));

			if(spiffsPartition) {
				Serial.print(_F("  ls - list files in spiffs\r\n"
								"  cat - show first file in spiffs\r\n"));
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

	Serial << _F("\r\nCurrently running ") << partition.name() << " @ 0x" << String(partition.address(), HEX) << '.'
		   << endl;
	Serial << _F("Type 'help' and press enter for instructions.") << endl << endl;

	Serial.onDataReceived(serialCallBack);
}
