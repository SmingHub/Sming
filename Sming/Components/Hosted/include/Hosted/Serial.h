/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Serial.h
 *
 * @author 2021 Slavey Karadzhov <slav@attachix.com>
 *
 *
 ****/

#pragma once

#ifndef ARCH_HOST
#error "Hosted::Serial can be used only on the Host architecture!"
#endif

#include <SerialLib.h>
#include <Stream.h>
#include <WString.h>
#include <hostlib/hostmsg.h>

namespace Hosted
{
class Serial : public Stream
{
public:
	Serial(const String& ttyDevice) : ttyDevice(ttyDevice)
	{
	}

	~Serial()
	{
		transport.closeDevice();
	}

	bool setDtr(bool on)
	{
		return transport.DTR(on);
	}

	bool setRts(bool on)
	{
		return transport.RTS(on);
	}

	/** @brief  Initialise the serial port
     *  @param  baud BAUD rate of the serial port (Default: 9600)
     */
	bool begin(uint32_t baud = 9600)
	{
		char result = transport.openDevice(ttyDevice.c_str(), baud);
		if(result == 1) {
			return true;
		}

		host_debug_w("Hosted::Serial:begin error: %d", result);
		return false;
	}

	size_t write(uint8_t c) override
	{
		if(transport.writeChar(c)) {
			return 1;
		}

		return 0;
	}

	int available() override
	{
		return transport.available();
	}

	int read() override
	{
		int ch;
		int result = transport.readChar(reinterpret_cast<char*>(&ch), 1);
		if(result == 1) {
			return ch;
		}

		return -1;
	}

	size_t readBytes(char* buffer, size_t length) override
	{
		int result = transport.readBytes(buffer, length, 100, 100);
		if(result > 0) {
			return result;
		}

		return 0;
	}

	size_t write(const uint8_t* buffer, size_t size)
	{
		char result = transport.writeBytes(buffer, size);
		if(result == 1) {
			return size;
		}

		return 0;
	}

	int peek() override
	{
		return -1;
	}

	void flush() override
	{
	}

private:
	String ttyDevice;

	serialib transport;
};

} // namespace Hosted
