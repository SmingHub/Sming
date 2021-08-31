/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Ethernet.cpp
 *
 ****/

#include "Ethernet.h"

String toString(EthernetEvent event)
{
	switch(event) {
#define XX(tag, desc)                                                                                                  \
	case EthernetEvent::tag:                                                                                           \
		return F(#tag);
		ETHERNET_EVENT_MAP(XX)
#undef XX
	default:
		return F("Unknown_") + unsigned(event);
	}
}

String toLongString(EthernetEvent event)
{
	switch(event) {
#define XX(tag, desc)                                                                                                  \
	case EthernetEvent::tag:                                                                                           \
		return F(desc);
		ETHERNET_EVENT_MAP(XX)
#undef XX
	default:
		return F("Unknown_") + unsigned(event);
	}
}
