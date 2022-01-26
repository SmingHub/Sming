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
class StationImpl : public StationClass, protected ISystemReadyHandler
{
public:
	StationImpl()
	{
		System.onReady(this);
	}

	void enable(bool enabled, bool save) override;
	bool isEnabled() const override;
	bool config(const String& ssid, const String& password, bool autoConnectOnStartup, bool save) override;
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

	// Called from WifiEventsImpl
	void eventHandler(esp_event_base_t base, int32_t id, void* data);

protected:
	void onSystemReady() override;

private:
	static void staticScanCompleted(wifi_event_sta_scan_done_t* event, uint8_t status);
#ifdef ENABLE_WPS
	static void staticWpsEventHandler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
	{
		auto self = static_cast<StationImpl*>(arg);
		self->wpsEventHandler(event_base, event_id, event_data);
	}
	void wpsEventHandler(esp_event_base_t event_base, int32_t event_id, void* event_data);
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
