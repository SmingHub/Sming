/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEventsImpl.cpp
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 */

#include "WifiEventsImpl.h"
#include <Platform/Station.h>

static WifiEventsImpl events;
WifiEventsClass& WifiEvents = events;

WifiEventsImpl::WifiEventsImpl()
{
}
