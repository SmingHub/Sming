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

#ifndef SYSTEM_INCLUDE_SYSDBG_H_
#define SYSTEM_INCLUDE_SYSDBG_H_

#include <user_config.h>

/** @brief callback function definition
 *  @param line text output by system, nul-terminated
 *  @note line points to buffer supplied in call to sysdbgInstallHook
 */
typedef std::function<void(const char* line, unsigned length)> SysdbgCallback;

/** @brief Provide a buffer and callback for monitoring system debug output
 *  @param callback Function to call when line has been output
 *  @param buffer Buffer to use for storing debug text
 *  @param bufferSize Size of buffer
 *  @retval bool true on success
 *  @note System messages are suppressed so the application can choose whether to display them
 *  or not via the callback.
 *
 *  Example code, typically in user's init() function:

	static char buffer[256];
	sysdbgInstallHook([](const char* line, unsigned length) {
		debug_w("[SYS] %s", line);
		if(memcmp(line, "E:M ", 4) == 0) {
			debug_print_stack();
		}
	}, buffer, sizeof(buffer));

 */
bool sysdbgInstallHook(SysdbgCallback callback, char* buffer, uint16_t bufferSize);

/** @brief Remove debug hook and revert to regular uart debug output */
void sysdbgRemoveHook();

#endif /* SYSTEM_INCLUDE_SYSDBG_H_ */
