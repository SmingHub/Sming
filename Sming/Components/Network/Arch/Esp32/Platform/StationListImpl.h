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
		auto err = esp_wifi_ap_get_sta_list(&list);
		if(err != ESP_OK) {
			debug_w("Can't get list of connected stations");
			return;
		}
		for(int i = 0; i < list.num; i++) {
			add(new Info{list.sta[i]});
		}
	}

private:
	class Info : public StationInfo
	{
	public:
		Info()
		{
		}

		Info(wifi_sta_info_t& info) : info(&info)
		{
		}

		MacAddress mac() const override
		{
			return info ? MacAddress{info->mac} : MacAddress{};
		}

		int8_t rssi() const override
		{
			return info ? info->rssi : 0;
		}

		// Note: ESP32 does not provide IP information
		IpAddress ip() const override
		{
			return IpAddress{};
		}

	private:
		wifi_sta_info_t* info{nullptr};
	};

	wifi_sta_list_t list{};
};
