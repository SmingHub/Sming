/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2017 by Slavey Karadzhov
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * OtaUpgradeStream.h
 *
 ****/

#ifdef ENABLE_OTA_ENCRYPTION
#include <EncryptedOtaUpgradeStream.h>
typedef EncryptedOtaUpgradeStream OtaUpgradeStream;
#else
#include <BasicOtaUpgradeStream.h>
typedef BasicOtaUpgradeStream OtaUpgradeStream;
#endif
