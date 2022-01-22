/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * emu.h
 *
 ****/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Executing this function will run once the main emulator loop.
 * @retval int Milliseconds until next due timer, -1 if none
 */
int host_main_loop();

#ifdef __cplusplus
}
#endif
