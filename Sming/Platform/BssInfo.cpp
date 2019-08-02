
#include "BssInfo.h"

String BssInfo::getAuthorizationMethodName() const
{
	switch(authorization) {
	case AUTH_OPEN:
		return F("OPEN");
	case AUTH_WEP:
		return F("WEP");
	case AUTH_WPA_PSK:
		return F("WPA_PSK");
	case AUTH_WPA2_PSK:
		return F("WPA2_PSK");
	case AUTH_WPA_WPA2_PSK:
		return F("WPA_WPA2_PSK");
	default:
		SYSTEM_ERROR("Unknown auth: %d", authorization);
		return nullptr;
	}
}

uint32_t BssInfo::getHashId() const
{
	uint32_t a = bssid[4] | (bssid[5] << 8);
	uint32_t b = bssid[0] | (bssid[1] << 8) | (bssid[2] << 16) | (bssid[3] << 24);
	return a ^ b;
}
