
#include "BssInfo.h"

String toString(WifiAuthMode mode)
{
	switch(mode) {
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
		return F("UNKNOWN_") + unsigned(mode);
	}
}
