/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationListImpl.cpp
 *
 ****/

#include "StationListImpl.h"
#include <esp_wifi.h>

StationListImpl::StationListImpl()
{
	err = esp_wifi_ap_get_sta_list(&list);
	if(err != ESP_OK) {
		debug_w("Can't get list of connected stations");
		list.num = 0;
	}
}
