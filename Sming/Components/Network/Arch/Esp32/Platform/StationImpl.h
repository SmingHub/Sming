/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationImpl.h - Esp32 WiFi Station
 *
 ****/

#pragma once

#include <Platform/Station.h>
#include <Platform/System.h>
#include <esp_wifi.h>

#ifdef ENABLE_SMART_CONFIG
#include <esp_smartconfig.h>
#include <memory>
#endif

struct esp_netif_obj;

namespace SmingInternal
{
namespace Network
{
class StationImpl : public StationClass
{
public:
	void enable(bool enabled, bool save) override;
	bool isEnabled() const override;
	bool config(const Config& cfg) override;
	bool connect() override;
	bool disconnect() override;
	StationConnectionStatus getConnectionStatus() const override;
	bool isEnabledDHCP() const override;
	void enableDHCP(bool enable) override;
	void setHostname(const String& hostname) override;
	String getHostname() const override;
	IpAddress getIP() const override;
	MacAddress getMacAddress() const override;
	bool setMacAddress(const MacAddress& addr) const override;
	IpAddress getNetworkMask() const override;
	IpAddress getNetworkGateway() const override;
	IpAddress getNetworkBroadcast() const override;
	bool setIP(IpAddress address, IpAddress netmask, IpAddress gateway) override;
	String getSSID() const override;
	MacAddress getBSSID() const override;
	String getPassword() const override;
	int8_t getRssi() const override;
	uint8_t getChannel() const override;
	bool startScan(ScanCompletedDelegate scanCompleted) override;

#ifdef ENABLE_SMART_CONFIG
	bool smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback) override;
	void smartConfigStop() override;
#endif

#ifdef ENABLE_WPS
	bool wpsConfigStart(WPSConfigDelegate callback) override;
	void wpsConfigStop() override;
#endif

	// Called from network event handler (init.cpp)
	void dispatchStaStart();
	void dispatchStaConnected(const wifi_event_sta_connected_t& event);
	void dispatchStaDisconnected(const wifi_event_sta_disconnected_t& event);

	void dispatchStaGotIp(const ip_event_got_ip_t&)
	{
		connectionStatus = eSCS_GotIP;
	}

	void dispatchStaLostIp()
	{
		connectionStatus = eSCS_Connecting;
	}

	void dispatchScanDone(const wifi_event_sta_scan_done_t& event);

#ifdef ENABLE_WPS
	void dispatchStaWpsErFailed();
	void dispatchStaWpsErTimeout();
	void dispatchStaWpsErPin();
	void dispatchWpsErSuccess(const wifi_event_sta_wps_er_success_t& event);
#endif

private:
#ifdef ENABLE_WPS
	bool wpsCallback(WpsStatus status);
	bool wpsConfigure(uint8_t credIndex);
#endif
#ifdef ENABLE_SMART_CONFIG
	void internalSmartConfig(smartconfig_event_t event, void* pdata);
	static void smartConfigEventHandler(void* arg, esp_event_base_t base, int32_t id, void* data);
#endif

private:
	StationConnectionStatus connectionStatus{eSCS_Idle};
	bool runScan{false};
#ifdef ENABLE_WPS
	struct WpsConfig;
	WpsConfig* wpsConfig;
#endif
#ifdef ENABLE_SMART_CONFIG
	std::unique_ptr<SmartConfigEventInfo> smartConfigEventInfo; ///< Set during smart handling
#endif
	esp_netif_obj* stationNetworkInterface{nullptr};
};

extern StationImpl station;

}; // namespace Network
}; // namespace SmingInternal
