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
		info.reset(wifi_softap_get_station_info());
		if(info) {
			cur = info.get();
		}
	}

	~StationListImpl()
	{
		wifi_softap_free_station_info();
	}

	void reset() override
	{
		if(info) {
			cur = info.get();
		}
	}

	bool next() override
	{
		if(cur != nullptr) {
			cur = STAILQ_NEXT(info, next);
		}
		return cur != nullptr;
	}

	MacAddress mac() const override
	{
		return cur ? cur->bssid : MacAddress{};
	}

	// Note: ESP8266 does not provide RSSI information
	int8_t rssi() const override
	{
		return 0;
	}

	IpAddress ip() const override
	{
		return cur ? IpAddress{cur->ip} : IpAddress{};
	}

private:
	std::unique_ptr<station_info> info;
	station_info* cur{nullptr};
	unsigned index{0};
};
