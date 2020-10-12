#include <Network/Mdns/Responder.h>
#include <Platform/System.h>

#if MDNS_LWIP >= 0x0200

extern "C" {
#ifndef LWIP_MDNS_RESPONDER
#define LWIP_MDNS_RESPONDER 1
#endif
#include <lwip/apps/mdns.h>
}

#ifdef ARCH_ESP8266
extern struct netif* netif_default_LWIP2;
#define netif_default netif_default_LWIP2
#endif

#else

extern "C" {
#if MDNS_LWIP > 0x0100
#include <lwip/mdns.h>
#endif

// Sets the service name
void mdns_set_name(const char* name);
}

#include <Platform/Station.h>

#endif

#define MDNS_TTL_DEFAULT 120

namespace mDNS
{
bool Responder::begin(const String& hostname)
{
#if MDNS_LWIP >= 0x0200

	mdns_resp_init();

#if MDNS_LWIP >= 0x0202
	err_t err = mdns_resp_add_netif(netif_default, hostname.c_str());
	if(err == ERR_OK) {
		System.queueCallback(InterruptCallback([]() { mdns_resp_announce(netif_default); }));
	}
#else
	err_t err = mdns_resp_add_netif(netif_default, hostname.c_str(), MDNS_TTL_DEFAULT);
#endif

	if(err != ERR_OK) {
		debug_e("mdns_resp_add_netif failed, err = %d", err);
		return false;
	}

#else

	this->hostname = hostname;

	mdns_info mi{};
	mi.host_name = this->hostname.begin();
	mi.server_name = const_cast<char*>("Sming");
	mi.ipAddr = WifiStation.getIP();

#ifdef ENABLE_ESPCONN
	espconn_mdns_init(&mi);
#else
	mdns_init(&mi);
#endif

#endif

	debug_i("MDNS initialised for '%s'", hostname.c_str());
	return true;
}

bool Responder::addService(Service& svc)
{
	auto si = svc.getInfo();
	debug_i("MDNS addService '%s'", si.name.c_str());

#if MDNS_LWIP >= 0x0200

	if(svc.id >= 0) {
		debug_e("MDNS: Service already in use");
		return false;
	}

	auto getSrvTxt = [](mdns_service* service, void* userdata) {
		auto txt = static_cast<Service*>(userdata)->getTxt();
		err_t res = 0;
		for(auto s : txt) {
			res |= mdns_resp_add_service_txtitem(service, s, strlen(s));
		}
		debug_i("mdns add service: res = %d", res);
	};

	String serviceType = String('_') + si.type;
#if MDNS_LWIP >= 0x0202
	auto id = mdns_resp_add_service(netif_default, si.name.c_str(), serviceType.c_str(), mdns_sd_proto(si.protocol),
									si.port, getSrvTxt, &svc);
#else
	auto id = mdns_resp_add_service(netif_default, si.name.c_str(), serviceType.c_str(), mdns_sd_proto(si.protocol),
									si.port, MDNS_TTL_DEFAULT, getSrvTxt, &svc);
#endif
	if(id < 0) {
		debug_w("mdns_resp_add_service failed, err = %d", id);
		return false;
	}

	debug_i("mdns_resp_add_service returned %d", id);

	svc.id = id;

#else

	// Service already registered?
	if(service != nullptr) {
		debug_e("MDNS: Service already registered");
		return false;
	}

	// Initialise text
	auto txt = svc.getTxt();
	if(txt.count() > ARRAY_SIZE(mdns_info::txt_data)) {
		debug_e("Too many MDNS TXT items");
		return false;
	}

	// Have to tear down then re-initialise
#ifdef ENABLE_ESPCONN
	espconn_mdns_close();
#else
	mdns_close();
#endif

	service = &svc;

	svc.type = std::move(si.type);
	svc.txt = std::move(txt);

	mdns_info mi{};
	mi.host_name = hostname.begin();
	mi.server_name = svc.type.begin();
	mi.ipAddr = WifiStation.getIP();
	mi.server_port = si.port;

	unsigned i = 0;
	for(auto s : svc.txt) {
		mi.txt_data[i++] = const_cast<char*>(s);
	}

#ifdef ENABLE_ESPCONN
	espconn_mdns_init(&mi);
#else
	mdns_init(&mi);
#endif

	// We can now set the service name
	mdns_set_name(si.name.c_str());
#endif

	return true;
}

void Responder::end()
{
#if MDNS_LWIP >= 0x0200

	mdns_resp_remove_netif(netif_default);

#else

#ifdef ENABLE_ESPCONN
	espconn_mdns_close();
#else
	mdns_close();
#endif

	if(service != nullptr) {
		service->type = nullptr;
		service->txt = nullptr;
		service = nullptr;
	}

#endif

	debug_i("MDNS stopped");
}

bool Responder::restart()
{
	debug_i("MDNS restarting...");

#if MDNS_LWIP >= 0x0202

	mdns_resp_restart(netif_default);
	return true;

#elif MDNS_LWIP >= 0x0200

	mdns_resp_netif_settings_changed(netif_default);
	return true;

#else

	auto svc = service;
	end();
	return svc ? addService(*svc) : begin(hostname);

#endif
}

} // namespace mDNS
