#pragma once

#include <HSPI/Device.h>

/*
 * Basic device to test chip selects, etc.
 */
class BasicDevice : public HSPI::Device
{
public:
	using Device::Device;

	HSPI::IoModes getSupportedIoModes() const override
	{
		return HSPI::IoMode::SPIHD;
	}

	bool begin(HSPI::PinSet pinSet, uint8_t chipSelect)
	{
		bool ok = Device::begin(pinSet, chipSelect);
		Serial.printf(_F("Init(%u): %s\r\n"), chipSelect, ok ? "OK" : "FAIL");
		return ok;
	}

	void write()
	{
		HSPI::Request req;
		req.setCommand8(0x11);
		req.setAddress(0x22334455, 32);
		req.out.set8(0x55);
		execute(req);
	}

	void read()
	{
		HSPI::Request req;
		req.setCommand8(0x66);
		req.setAddress24(0x778899);
		req.in.set8(0);
		execute(req);
	}

	void writeAsync()
	{
		//		auto req = new Request;
		//		req.setCommand8(0x01); // WRMR
		//		req.out.set8(uint8_t(mode));
		//		execute(req);
	}

	void readAsync()
	{
		//		Request req;
		//		req.setCommand8(0x05); // RDMR
		//		req.in.set8(0);
		//		execute(req);
	}
};
