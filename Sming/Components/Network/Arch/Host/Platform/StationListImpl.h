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
#include <Data/Range.h>

class StationListImpl : public StationList
{
public:
	StationListImpl()
	{
		for(unsigned i = 0; i < 5; ++i) {
			add(new Info(i));
		}
	}

private:
	class Info : public StationInfo
	{
	public:
		Info(int index) : index(index)
		{
		}

		MacAddress mac() const override
		{
			return (index < 0) ? MacAddress{} : MacAddress({5, 4, 3, 2, 1, uint8_t(index)});
		}

		int8_t rssi() const override
		{
			return (index < 0) ? 0 : TRange<int8_t>(-90, -20).random();
		}

		IpAddress ip() const override
		{
			return (index < 0) ? IpAddress{} : IpAddress(10, 0, 0, 100 + index);
		}

	private:
		int index;
	};
};
