/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OsMessageInterceptor.h -  * Support for intercepting OS debug output (calls to os_printf, etc.)
 *
 * author mikee47 <mike@sillyhouse.net> Feb 2019
 *
 * Provided to assist with tracking down the origin of error messages output by SDK.
 * This is usually done in conjunction with the debugger or by performing a stack dump
 * to establish the origin of the problem code.
 *
 * Example usage:

	static OsMessageInterceptor osMessageInterceptor;

	void handleOsMessage(OsMessage& msg)
	{
		debug_w("[OS] %s", msg.getBuffer());
		if(msg.startsWith("E:M ")) {
			// Handle memory error
		} else if(msg.contains(" assert ")) {
			// Handle 'assert' message
		}
	}

	void init()
	{
		...

		osMessageInterceptor.begin(handleOsMessage);

		// Example of 'bad' system call, generates "E:M 0" message
		os_malloc(0);

		...
	}

 *
 *
 ****/

#pragma once

#include "Data/Buffer/LineBuffer.h"

/**
 * @brief Fixed-size buffer for OS messages
 */
typedef LineBuffer<128> OsMessage;

/**
 * @brief Callback to receive OS message line
 * @param msg The message
 * @note We don't use std::function because it produces a more elaborate stack trace without any real benefit
 */
typedef void (*OsMessageCallback)(OsMessage& message);

/**
 * @brief Class to handle interception of OS messages
 * @note Messages are output character-by-character so must be reassembled into lines
 * before passing to callback function
 */
class OsMessageInterceptor
{
public:
	~OsMessageInterceptor()
	{
		end();
	}

	/**
	 * @brief Enable message interception
	 * @param callback Function to receive messages
	 * @note Messages are suppressed from output during interception. If you wish to send them to
	 * the uart, etc. then do this in the callback.
	 */
	void begin(OsMessageCallback callback);

	/**
	 * @brief Stop message interception and revert to output via uart driver
	 */
	void end();

protected:
	void putc(char c);

	static void static_putc(char c)
	{
		self->putc(c);
	}

private:
	OsMessage message;
	OsMessageCallback callback = nullptr; ///< User-provided callback
	static OsMessageInterceptor* self;
};
