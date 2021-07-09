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
		err = esp_wifi_ap_get_sta_list(&list);
		if(err != ESP_OK) {
			debug_w("Can't get list of connected stations");
		}
	}

	void reset() override
	{
		index = 0;
	}

	bool next() override
	{
		++index;
		return isValid();
	}

	MacAddress mac() const override
	{
		if(!isValid()) {
			return MacAddress{};
		}
		return list.sta[index].mac;
	}

	int8_t rssi() const override
	{
		if(!isValid()) {
			return 0;
		}
		return list.sta[index].rssi;
	}

	// Note: ESP32 does not provide IP information
	IpAddress ip() const override
	{
		return IpAddress{};
	}

private:
	bool isValid() const
	{
		return int(index) < list.num;
	}

	wifi_sta_list_t list{};
	esp_err_t err;
	unsigned index{0};
};
