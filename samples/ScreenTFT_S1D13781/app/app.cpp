#include <SmingCore.h>
#include <Platform/Timers.h>
#include <demo.h>
#include <HSPI/RAM/PSRAM64.h>
#include <HSPI/RAM/IS62-65.h>
#include <BasicDevice.h>
#include <Services/Profiling/CpuUsage.h>
#include <HSPI/StreamAdapter.h>

namespace
{
HSPI::Controller spi;
S1D13781::Gfx graphics(spi);
DisplayDemo demo(graphics);
HSPI::RAM::PSRAM64 ram(spi);
//HSPI::RAM::IS62_65 ram(spi);
SimpleTimer ramTimer;
BasicDevice dev0(spi), dev1(spi), dev2(spi);
Profiling::CpuUsage cpuUsage;

// Chip selects to use for our devices
#define CS_LCD 0
#define CS_RAM 2

void printSpiStats()
{
	auto& stats = spi.stats;
	debug_i("HSPI stats: requests = %u, trans = %u, waitCycles = %u, tasks queued = %u, tasks cancelled = %u",
			stats.requestCount, stats.transCount, stats.waitCycles, stats.tasksQueued, stats.tasksCancelled);
	debug_i("Cpu Usage: %0.2f %%", cpuUsage.getUtilisation() / 100.0);

	stats.clear();
}

String getSpeed(NanoTime::Time<uint32_t> time, uint32_t byteCount)
{
	String s;
	s += time.toString();
	s += " (";
	auto mbps = 8 * 1e6 * byteCount / time / (1024 * 1024);
	s += mbps;
	s += "Mbit/s)";
	return s;
}

uint32_t countDiffs(const uint8_t* p1, const uint8_t* p2, size_t length)
{
	uint32_t count{0};
	for(size_t i = 0; i < length; ++i) {
		if(p1[i] != p2[i]) {
			++count;
		}
	}
	return count;
}

void memTest()
{
	Serial.print(_F("Testing memory: IO Mode = "));
	auto& info = HSPI::getIoModeInfo(ram.getIoMode());
	Serial.print(*info.name);
	Serial.print(_F(", speed = "));
	Serial.println(ram.getSpeed());

	const unsigned blockSize{1024};
	//	const unsigned ramSize = ram.getSize();
	const unsigned ramSize = std::min(0x100000U, ram.getSize());

	auto wrData = new uint8_t[blockSize];
	auto rdData = new uint8_t[blockSize];
	auto zero = new uint8_t[blockSize];

	memset(zero, 0, blockSize);

	for(unsigned i = 0; i < blockSize; ++i) {
		wrData[i] = i + 0x50;
	}

	//	HSPI::Request req;
	//	req.setCommand8(0xC0);
	//	ram.execute(req);

	ram.readId();

	auto mode = ram.getIoMode();

	//if(mode == HSPI::IoMode::QIO) {
	//	HSPI::Request req;
	//	req.setCommand8(0xC0);
	//	ram.execute(req);
	//}

	bool ok{true};
	ElapseTimer timer;
	NanoTime::Time<uint32_t> writeTime(NanoTime::Microseconds, 0);
	NanoTime::Time<uint32_t> readTime(NanoTime::Microseconds, 0);
	uint32_t startAddress{0x1000};
	uint32_t address = startAddress;
	uint32_t failCount{0};
	for(; address < ramSize; address += blockSize) {
		//		os_get_random(wrData, blockSize);

		// Write
		ElapseTimer tmr;
		tmr.start();
		ram.write(address, wrData, blockSize);
		writeTime += tmr.elapsedTime();

		// Read
		ram.setIoMode(HSPI::IoMode::SPIHD);
		memset(rdData, 0, blockSize);
		tmr.start();
		ram.read(address, rdData, blockSize);
		readTime += tmr.elapsedTime();
		ram.setIoMode(mode);

		//ram.write(address, zero, blockSize);

		// Compare
		auto diffCount = countDiffs(wrData, rdData, blockSize);

		if(diffCount != 0) {
			ok = false;

			if(failCount == 0) {
				debug_e("memTest failed at 0x%08x", address);
				auto len = std::min(blockSize, 256U);
				debug_hex(ERR, "WR", wrData, len);
				debug_hex(ERR, "RD", rdData, len);
			}
			//									break;

			failCount += diffCount;
		}

		WDT.alive();
	}

	delete[] rdData;
	delete[] wrData;

	//if(mode == HSPI::IoMode::QIO) {
	//	HSPI::Request req;
	//	req.setCommand8(0xC0);
	//	ram.execute(req);
	//}

	if(ok) {
		debug_i("memTest complete");
	} else {
		debug_w("Mismatch in %u bytes (%f%%)", failCount, 100.0 * failCount / ramSize);
	}

	debug_i("Elapsed: %s, I/O write: %s, I/O read: %s", timer.elapsedTime().toString().c_str(),
			getSpeed(writeTime, address - startAddress).c_str(), getSpeed(readTime, address).c_str());

	printSpiStats();
}

void spiRamTest()
{
	debug_i("SPI RAM test");

	for(unsigned i = 0; i < 8; ++i) {
		auto m = HSPI::IoMode(i);
		if(!ram.isSupported(m)) {
			continue;
		}
		assert(ram.setIoMode(m));
		memTest();
	}

	/*
	// Write memory
	char buf[256];
	flash_s1d13781text.readFlash(0, buf, sizeof(buf));
	ram.write(0, buf, sizeof(buf));
	debug_hex(ERR, "WRITE", buf, sizeof(buf));

	// Read memory
	memset(buf, 0, sizeof(buf));
	ram.read(0, buf, sizeof(buf));
	debug_hex(ERR, "READ", buf, sizeof(buf));
*/
}

void basicTests(HSPI::PinSet pinSet)
{
	dev0.begin(pinSet, 0);
	dev1.begin(pinSet, 1);
	dev2.begin(pinSet, 2);

	dev0.write();
	dev0.read();
	dev1.write();
	dev1.read();
	dev2.write();
	dev2.read();

	dev2.end();
	dev1.end();
	dev0.end();
}

void basicTests()
{
	//	basicTests(HSPI::PinSet::overlap);
	basicTests(HSPI::PinSet::normal);
}

ElapseTimer streamTimer;
HSPI::StreamAdapter adapter(ram);

void printAdapterStats()
{
	auto requested = adapter.getBytesRequested();
	auto transferred = adapter.getBytesTransferred();
	String speed = getSpeed(streamTimer.elapsedTime(), transferred);
	debug_w("Stream %s all done, %u bytes requested, %u bytes transferred, elapsed = %s",
			adapter.getIsWrite() ? "WRITE" : "READ", requested, transferred, speed.c_str());
	debug_w("Max tasks = %u", System.getMaxTaskCount());
}

void ramStreamTest()
{
	cpuUsage.reset();
	streamTimer.start();
	auto stream = new FSTR::Stream(smingLogo);
	adapter.write(stream, 0, 409600, []() {
		printAdapterStats();
		printSpiStats();

		auto len = adapter.getBytesTransferred();
		auto fs = new FileStream("logo.out", eFO_CreateNewAlways | eFO_ReadWrite);
		streamTimer.start();
		adapter.read(fs, 0, len, []() {
			printAdapterStats();
			printSpiStats();
			//			gfx.setRotation(Window::main, 0);
			demo.start();
		});
	});
}

void ready()
{
	spiRamTest();
	ramStreamTest();
}

} // namespace

void init()
{
// Start serial for serial monitor
// CS #1 conflicts with regular serial TX pin, so switch to secondary debug
#if !defined(ENABLE_GDB) && !defined(ARCH_HOST) && (CS_LCD == 1 || CS_RAM == 1)
	Serial.setPort(UART_ID_1);
#endif
	Serial.setTxBufferSize(2048);
	Serial.setTxWait(true);
	Serial.begin(SERIAL_BAUD_RATE);
	Serial.systemDebugOutput(true);

	spiffs_mount();

	spi.begin();

	// Initialise display and start demo
	graphics.setSpeed(10000000U);
	debug_i("LCD clock = %u", graphics.getSpeed());

	if(!graphics.begin(HSPI::PinSet::overlap, CS_LCD)) {
		debug_e("Failed to start LCD device");
	} else {
		demo.drawLogo();
	}

	// Initialise RAM
	ram.setSpeed(40000000U);
	if(!ram.begin(HSPI::PinSet::overlap, CS_RAM)) {
		debug_e("Failed to start RAM device");
	}
	debug_i("RAM clock = %u", ram.getSpeed());

	cpuUsage.begin(ready);
}
