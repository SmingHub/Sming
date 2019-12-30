/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Alert.cpp
 *
 ****/

#include <Network/Ssl/Alert.h>
#include <FlashString/Map.hpp>

namespace Ssl
{
#define XX(tag, code) DEFINE_FSTR_LOCAL(alertStr_##tag, #tag)
SSL_ALERT_CODE_MAP(XX)
#undef XX

#define XX(tag, code) {Alert::tag, &alertStr_##tag},
DEFINE_FSTR_MAP_LOCAL(alertCodeMap, Alert, FSTR::String, SSL_ALERT_CODE_MAP(XX));
#undef XX

String getAlertString(Alert alert)
{
	auto s = String(alertCodeMap[alert]);
	return s ?: F("ALERT_") + String(unsigned(alert));
}

} // namespace Ssl
