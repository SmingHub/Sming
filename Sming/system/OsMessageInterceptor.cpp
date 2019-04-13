/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OsMessageInterceptor.h
 *
 * author mikee47 <mike@sillyhouse.net> Feb 2019
 *
 ****/

#include <OsMessageInterceptor.h>

extern void uart_debug_putc(char);

OsMessageInterceptor* OsMessageInterceptor::self;

void OsMessageInterceptor::putc(char c)
{
	c = message.addChar(c);
	if(c == '\n' && callback) {
		callback(message);
		message.clear();
	}
}

void OsMessageInterceptor::begin(OsMessageCallback callback)
{
	this->callback = callback;
	message.clear();
	self = this;
	ets_install_putc1(static_putc);
	system_set_os_print(true);
}

void OsMessageInterceptor::end()
{
	ets_install_putc1(uart_debug_putc);
}
