#include <SmingCore.h>
#include <Storage/SysMem.h>
#include <Storage/ProgMem.h>
#include <Storage/Debug.h>
#include <Platform/Timers.h>

IMPORT_FSTR(FS_app, PROJECT_DIR "/app/application.cpp")

void listSpiffsPartitions()
{
	Serial.println(_F("** Enumerate registered SPIFFS partitions"));
	for(auto it = Storage::findPartition(Storage::Partition::SubType::Data::spiffs); it; ++it) {
		Serial.print(F(">> Mounting '"));
		Serial.print((*it).name());
		Serial.println("' ...");
		bool ok = spiffs_mount(*it);
		Serial.println(ok ? "OK, listing files:" : "Mount failed!");
		if(ok) {
			Directory dir;
			if(dir.open()) {
				while(dir.next()) {
					Serial.print("  ");
					Serial.println(dir.stat().name);
				}
			}
			Serial.print(dir.count());
			Serial.println(F(" files found"));
			Serial.println();
		}
	}
}

void printPart(Storage::Partition part)
{
	size_t bufSize = std::min(4096U, part.size());
	char buf[bufSize];
	OneShotFastUs timer;
	if(!part.read(0, buf, bufSize)) {
		debug_e("Error reading from partition '%s'", part.name().c_str());
	} else {
		auto elapsed = timer.elapsedTime();
		String s = part.getDeviceName();
		s += '/';
		s += part.name();
		m_printHex(s.c_str(), buf, std::min(128U, bufSize));
		m_printf(_F("Elapsed: %s\r\n"), elapsed.toString().c_str());
		if(elapsed != 0) {
			m_printf(_F("Speed:   %u KB/s\r\n\r\n"), 1000 * bufSize / elapsed);
		}
	}
	Serial.println();
}

void printPart(const String& partitionName)
{
	auto part = Storage::findPartition(partitionName);
	if(!part) {
		debug_e("Partition '%s' not found", partitionName.c_str());
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
	part = Storage::sysMem.createPartition(F("fs_app"), FS_app, Storage::Partition::Type::data, 100);
	printPart(part);
	printPart(part);
	printPart(part);

	Serial.println(_F("** Reading SysMem device (RAM)"));
	part = Storage::sysMem.createPartition(F("fs_app"), FS_app, Storage::Partition::Type::data, 100);
	printPart(part);
	printPart(part);
	printPart(part);

	Serial.println(_F("** Reading ProgMem device"));
	part = Storage::progMem.createPartition(F("fs_app"), FS_app, Storage::Partition::Type::data, 100);
	printPart(part);
	printPart(part);
	printPart(part);
}
