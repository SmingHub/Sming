/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * sysdbg.h
 *
 * author mikee47 <mike@sillyhouse.net> Feb 2019
 *
 * Support for intercepting SDK debug output
 *
 ****/

#include "sysdbg.h"

extern void uart_debug_putc(char);

struct SystemDebug {
	SysdbgCallback callback = nullptr; ///< User-provided callback
	char* buffer = nullptr;			   ///< User-provided buffer
	uint16_t size = 0;				   ///< Number of bytes available in buffer
	uint16_t length = 0;			   ///< Number of characters stored

	bool init(SysdbgCallback callback, char* buffer, uint16_t bufferSize)
	{
		this->callback = callback;
		this->buffer = buffer;
		size = bufferSize;
		length = 0;
		return callback != nullptr && buffer != nullptr && bufferSize != 0;
	}

	void putc(char c)
	{
		if(callback == nullptr) {
			uart_debug_putc(c);
			return;
		}

		if(c == '\n') {
			if(length > 0) {
				buffer[length] = '\0';
				// Any system output generated via callback will be written directly
				auto cb = callback;
				callback = nullptr;
				cb(buffer, length);
				callback = cb;
				length = 0;
			}
		} else if((c != '\r') && ((length + 1) < size)) {
			buffer[length++] = c;
		}
	}
};

static SystemDebug sysdbg;

bool sysdbgInstallHook(SysdbgCallback callback, char* buffer, uint16_t bufferSize)
{
	if(sysdbg.init(callback, buffer, bufferSize)) {
		ets_install_putc1([](char c) { sysdbg.putc(c); });
		return true;
	} else {
		return false;
	}
}

void sysdbgRemoveHook()
{
	ets_install_putc1(uart_debug_putc);
	sysdbg.init(nullptr, nullptr, 0);
}
