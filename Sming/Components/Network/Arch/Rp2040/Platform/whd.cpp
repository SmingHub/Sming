/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * whd.cpp
 *
 ****/

#include "whd.h"
#include <pico/cyw43_arch.h>

String whd_get_ssid(int itf)
{
	wlc_ssid_t ssid;
	int err = cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_SSID, sizeof(ssid), reinterpret_cast<uint8_t*>(&ssid), itf);
	return err ? nullptr : String(reinterpret_cast<const char*>(ssid.SSID), ssid.SSID_len);
}

MacAddress whd_get_bssid(int itf)
{
	MacAddress addr;
	cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_BSSID, sizeof(addr), &addr[0], itf);
	return addr;
}

uint8_t whd_get_channel(int itf)
{
	channel_info_t info{};
	cyw43_ioctl(&cyw43_state, CYW43_IOCTL_GET_CHANNEL, sizeof(info), reinterpret_cast<uint8_t*>(&info), itf);
	return info.hw_channel;
}
