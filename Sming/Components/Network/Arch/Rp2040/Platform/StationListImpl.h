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
		add(new Info{});
	}

private:
	class Info : public StationInfo
	{
	public:
		Info()
		{
		}

		MacAddress mac() const override
		{
			return MacAddress{};
		}

		int8_t rssi() const override
		{
			return 0;
		}

		IpAddress ip() const override
		{
			return IpAddress{};
		}
	};
};
