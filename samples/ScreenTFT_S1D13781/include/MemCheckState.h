#pragma once

#include <S1D13781/Driver.h>
#include <S1D13781/registers.h>

/*
 * B: Build outgoing block
 * C: Check received block
 * W: Start write of outgoing block
 * R: Start read of incoming block
 *
 * CPU   B, B     C, B    C, B
 *       |        ^       ^
 *       |        |       |
 * SPI   -> W-> R-> W-> R-> W...
 *
 *
 * Interrupt version, 433ms
 * Additional time due to setup code. A 512 byte buffer creates 772 requests, for each of write/read,
 * a total of 1544 requests. So the additional (433 - 383) = 50ms is from 32us per request.
 * 1024 byte: 443ms
 * 256 byte: 423ms
 * 128 byte: 404ms
 * 64 bytes: ms FAIL
 *
 * With task queue and packet chaining:
 *
 * 1024 byte: 387ms
 * 512 byte: 391ms
 * 256 byte: 400ms
 * 128 byte: 418ms
 * 64 byte: 452ms
 *
 * So how come it's faster? Who cares - awesome, way to go :-)
 *
 */
class MemCheckState
{
public:
	MemCheckState(S1D13781::Driver& driver) : driver(driver)
	{
	}

	void execute()
	{
		driver.controller.stats.clear();
		buildBlock();
		writeBlock();
		readBlock();
		buildBlock();
	}

	InterruptDelegate onComplete;

private:
	void buildBlock()
	{
		// LUT MSB isn't used, always reads 0
		uint32_t mask = (writeAddr < S1D13781_LUT1_BASE) ? 0xFFFFFFFF : 0x00FFFFFF;
		auto buf = writeBuffer[bufIndex];
		for(unsigned i = 0; i < ARRAY_SIZE(writeBuffer[0]); ++i) {
			//			buf[i] = writeAddr;
			buf[i] = os_random() & mask;
		}
	}

	void writeBlock()
	{
		driver.write(reqWr, writeAddr, writeBuffer[bufIndex], bufSize);
		writeAddr += bufSize;
		bufIndex = 1 - bufIndex;
	}

	void readBlock()
	{
		auto callback = [](HSPI::Request& request) {
			System.queueCallback([](void* param) { static_cast<MemCheckState*>(param)->blockRead(); }, request.param);
		};
		driver.read(reqRd, readAddr, readBuffer, bufSize, callback, this);
	}

	void blockRead()
	{
		if(writeAddr < maxAddr) {
			writeBlock();
		} else {
			bufIndex = 1 - bufIndex;
		}

		checkBlock();
		readAddr += bufSize;
		if(readAddr < maxAddr) {
			readBlock();
			if(writeAddr < maxAddr) {
				buildBlock();
			}
		} else {
			complete();
		}
	}

	void checkBlock()
	{
		if(memcmp(readBuffer, writeBuffer[bufIndex], bufSize)) {
			debug_e("Mem check failed between 0x%08x and 0x%08x", readAddr, readAddr + bufSize - 1);

			unsigned checklen = ARRAY_SIZE(readBuffer);
			for(unsigned i = 0; i < checklen; ++i) {
				auto in = readBuffer[i];
				auto out = writeBuffer[bufIndex][i];
				if(in != out) {
					debug_e("  @ 0x%08x: out 0x%08x in 0x%08x", readAddr + (i * 4), out, in);
					break;
				}
			}
		}
	}

	void complete()
	{
		auto& stats = driver.controller.stats;
		debug_i("Memory check complete, %s, waitCycles = %u, trans = %u", timer.elapsedTime().toString().c_str(),
				stats.waitCycles, stats.transCount);

		debug_i("out = %u, in = %u", reqWr.busy, reqRd.busy);

		auto callback = onComplete;

		delete this;

		if(callback) {
			callback();
		}
	}

private:
	S1D13781::Driver& driver;
	const uint32_t maxAddr{S1D13781_REG_BASE};
	static const unsigned bufSize{512};
	uint32_t writeBuffer[2][bufSize / 4];
	uint32_t readBuffer[bufSize / 4];
	ElapseTimer timer;
	uint32_t writeAddr{0};
	uint32_t readAddr{0};
	uint8_t bufIndex{0};
	HSPI::Request reqRd, reqWr;
};
