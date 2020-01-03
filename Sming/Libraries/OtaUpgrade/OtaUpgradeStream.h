/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaUpgradeStream.h - Front-end header for OTA upgrade functionality, to be included by application code.
 *
 ****/

#ifdef ENABLE_OTA_ENCRYPTION
#include <EncryptedOtaUpgradeStream.h>
using OtaUpgradeStream = EncryptedOtaUpgradeStream;
#else
#include <BasicOtaUpgradeStream.h>
using OtaUpgradeStream = BasicOtaUpgradeStream;
#endif

/**
 * @typedef OtaUpgradeStream
 * 
 * Alias for either `BasicOtaUpgradeStream` or `EncryptedOtaUpgradeStream`, depending on encryption settings.
 * 
 * Application code should use this alias to avoid source code modifications when changing OTA upgrade security settings.
 */
