/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * init.cpp
 */

#include "StationImpl.h"
#include "AccessPointImpl.h"
#include "WifiEventsImpl.h"
#include <Platform/System.h>
#include <pico/cyw43_arch.h>

#ifdef DISABLE_WIFI

void rp2040_network_initialise()
{
}

void rp2040_network_service()
{
}

#else

// Called from startup
void rp2040_network_initialise()
{
	int err = cyw43_arch_init();
	if(err != 0) {
		return;
	}

#ifdef ENABLE_WIFI_DEBUG
	cyw43_state.trace_flags = 0xff;
#endif
}

void rp2040_network_service()
{
	cyw43_arch_poll();
}

extern "C" void __wrap_cyw43_cb_process_async_event(cyw43_t* self, const cyw43_async_event_t* ev)
{
	using namespace SmingInternal::Network;

	auto& msg = *reinterpret_cast<const whd_event_msg*>(ev);
	const_cast<whd_event_msg&>(msg).datalen = __builtin_bswap32(msg.datalen);

	EventInfo info{*self, msg};
	if(msg.ifidx == CYW43_ITF_STA) {
		station.eventHandler(info);
	} else if(msg.ifidx == CYW43_ITF_AP) {
		accessPoint.eventHandler(info);
	}
	events.eventHandler(info);

	const_cast<whd_event_msg&>(msg).datalen = __builtin_bswap32(msg.datalen);

	extern void __real_cyw43_cb_process_async_event(void*, const void*);
	__real_cyw43_cb_process_async_event(self, ev);
}

#endif
