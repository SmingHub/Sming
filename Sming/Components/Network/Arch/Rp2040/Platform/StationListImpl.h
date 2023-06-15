/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationListImpl.h
 *
 ****/

#pragma once

#include <Platform/StationList.h>

class StationListImpl : public StationList
{
public:
	StationListImpl()
	{
		int num_sta{0};
		MacAddress maclist[16];
		cyw43_wifi_ap_get_stas(&cyw43_state, &num_sta, &maclist[0][0]);
		for(int i = 0; i < num_sta; ++i) {
			add(new Info{maclist[i]});
		}
	}

private:
	class Info : public StationInfo
	{
	public:
		Info(MacAddress mac) : macaddr(mac)
		{
		}

		MacAddress mac() const override
		{
			return this->macaddr;
		}

		int8_t rssi() const override
		{
			return 0;
		}

		IpAddress ip() const override
		{
			return IpAddress{};
		}

	private:
		MacAddress macaddr;
	};
};
