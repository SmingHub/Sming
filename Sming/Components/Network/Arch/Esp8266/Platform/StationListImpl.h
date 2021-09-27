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
#include <esp_wifi.h>

class StationListImpl : public StationList
{
public:
	StationListImpl()
	{
		info.reset(wifi_softap_get_station_info());
		auto cur = info.get();
		while(cur != nullptr) {
			add(new Info{cur});
		}
	}

	~StationListImpl()
	{
		wifi_softap_free_station_info();
	}

private:
	class Info : public StationInfo
	{
	public:
		Info(station_info* info) : info(info)
		{
		}

		MacAddress mac() const override
		{
			return info ? MacAddress{info->bssid} : MacAddress{};
		}

		// Note: ESP8266 does not provide RSSI information
		int8_t rssi() const override
		{
			return 0;
		}

		IpAddress ip() const override
		{
			return info ? IpAddress{info->ip} : IpAddress{};
		}

	private:
		station_info* info;
	};

	std::unique_ptr<station_info> info;
};
