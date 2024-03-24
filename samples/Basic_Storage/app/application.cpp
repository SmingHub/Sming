#include <SmingCore.h>
#include <Storage/SysMem.h>
#include <Storage/ProgMem.h>
#include <Storage/Debug.h>

IMPORT_FSTR(FS_app, PROJECT_DIR "/app/application.cpp")

void listSpiffsPartitions()
{
	Serial.println(_F("** Enumerate registered SPIFFS partitions"));
	for(auto part : Storage::findPartition(Storage::Partition::SubType::Data::spiffs)) {
		Serial << _F(">> Mounting '") << part.name() << "' ..." << endl;
		bool ok = spiffs_mount(part);
		Serial.println(ok ? "OK, listing files:" : "Mount failed!");
		if(!ok) {
			continue;
		}

		Directory dir;
		if(dir.open()) {
			while(dir.next()) {
				Serial.print("  ");
				Serial.println(dir.stat().name);
			}
		}
		Serial << dir.count() << _F(" files found") << endl << endl;
	}
}

void printPart(Storage::Partition part)
{
	size_t bufSize = std::min(storage_size_t(4096), part.size());
	char buf[bufSize];
	OneShotFastUs timer;
	if(!part.read(0, buf, bufSize)) {
		Serial << _F("Error reading from partition '") << part.name() << "'" << endl;
	} else {
		auto elapsed = timer.elapsedTime();
		String s = part.getDeviceName();
		s += '/';
		s += part.name();
		m_printHex(s.c_str(), buf, std::min(128U, bufSize));
		Serial << _F("Elapsed: ") << elapsed.toString() << endl;
		if(elapsed != 0) {
			Serial << _F("Speed:   ") << 1000 * bufSize / elapsed << " KB/s" << endl;
		}
	}
	Serial.println();
}

void printPart(const String& partitionName)
{
	auto part = Storage::findPartition(partitionName);
	if(!part) {
		Serial << _F("Partition '") << partitionName << _F("' not found") << endl;
	} else {
		printPart(part);
	}
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	Storage::Debug::listDevices(Serial);

	listSpiffsPartitions();

	printPart(F("user0"));

	auto part = Storage::findPartition(F("user1"));
	printPart(part);
	Serial.println(_F("Writing some stuff to partition..."));
	String s = F("Some test stuff to write...");
	part.write(32, s.c_str(), s.length() + 1);
	uint8_t buf[32];
	os_get_random(buf, sizeof buf);
	part.write(64, buf, sizeof buf);
	printPart(part);

	Serial.println(_F("** Reading tests, repeat 3 times to show effect of caching (if any)"));

	Serial.println(_F("** Reading SysMem device (flash)"));
	part = Storage::sysMem.editablePartitions().add(F("fs_app FLASH"), FS_app, {Storage::Partition::Type::data, 100});
	printPart(part);
	printPart(part);
	printPart(part);

	Serial.println(_F("** Reading SysMem device (RAM)"));
	part = Storage::sysMem.editablePartitions().add(F("fs_app RAM"), FS_app, {Storage::Partition::Type::data, 100});
	printPart(part);
	printPart(part);
	printPart(part);

	Serial.println(_F("** Reading ProgMem device"));
	part =
		Storage::progMem.editablePartitions().add(F("fs_app PROGMEM"), FS_app, {Storage::Partition::Type::data, 100});
	printPart(part);
	printPart(part);
	printPart(part);
}
