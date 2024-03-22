#include <SmingCore.h>
#include <Ota/Network/HttpUpgrader.h>
#include <Storage/PartitionStream.h>
#include <Storage/SpiFlash.h>
#include <Ota/Upgrader.h>
#include <Data/Buffer/LineBuffer.h>

// If you want, you can define WiFi settings globally in Eclipse Environment Variables
#ifndef WIFI_SSID
#define WIFI_SSID "PleaseEnterSSID" // Put your SSID and password here
#define WIFI_PWD "PleaseEnterPass"
#endif

namespace
{
std::unique_ptr<Ota::Network::HttpUpgrader> otaUpdater;
Storage::Partition spiffsPartition;
OtaUpgrader ota;
LineBuffer<16> commandBuffer;

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
	otaUpdater = std::make_unique<Ota::Network::HttpUpgrader>();

	// select rom slot to flash
	auto part = ota.getNextBootPartition();

	/*
	 * Applications should always include a sanity check to ensure partitions being updated are
	 * not in use. This should always included the application partition but should also consider
	 * filing system partitions, etc. which may be actively in use.
	 */
	if(part == ota.getRunningPartition()) {
		Serial << F("May be running in temporary mode. Please reboot and try again.") << endl;
		return;
	}

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
		otaUpdater->addItem(SPIFFS_URL, spiffsPart,
							new Storage::PartitionStream(spiffsPart, Storage::Mode::BlockErase));
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

void showPrompt()
{
	Serial << _F("OTA> ") << commandBuffer;
}

void handleCommand(const String& str)
{
	if(F("connect") == str) {
		Serial << _F("Connecting to '") << WIFI_SSID << "'..." << endl;
		WifiStation.config(WIFI_SSID, WIFI_PWD);
		WifiStation.enable(true);
		WifiStation.connect();
		return;
	}

	if(F("ip") == str) {
		Serial << "ip: " << WifiStation.getIP() << ", mac: " << WifiStation.getMacAddress() << endl;
		return;
	}

	if(F("ota") == str) {
		doUpgrade();
		return;
	}

	if(F("switch") == str) {
		doSwitch();
		return;
	}

	if(F("restart") == str) {
		System.restart();
		return;
	}

	if(F("ls") == str) {
		Directory dir;
		if(dir.open()) {
			while(dir.next()) {
				Serial << "  " << dir.stat().name << endl;
			}
		}
		Serial << _F("filecount ") << dir.count() << endl;
		return;
	}

	if(F("cat") == str) {
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
		return;
	}

	if(F("info") == str) {
		showInfo();
		return;
	}

	if(F("help") == str) {
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
		return;
	}

	Serial << _F("unknown command: ") << str << endl;
}

void serialCallBack(Stream& stream, char arrivedChar, unsigned short availableCharsCount)
{
	switch(commandBuffer.process(stream, Serial)) {
	case commandBuffer.Action::submit:
		if(commandBuffer) {
			handleCommand(String(commandBuffer));
			commandBuffer.clear();
		}
		showPrompt();
		break;
	case commandBuffer.Action::clear:
		showPrompt();
		break;
	default:;
	}
}

} // namespace

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
	WifiEvents.onStationGotIP([](IpAddress ip, IpAddress netmask, IpAddress gateway) { showPrompt(); });

	Serial << endl
		   << _F("Currently running ") << partition.name() << " @ 0x" << String(partition.address(), HEX) << '.' << endl
		   << _F("Type 'help' and press enter for instructions.") << endl
		   << endl;

	showPrompt();

	Serial.onDataReceived(serialCallBack);
}
