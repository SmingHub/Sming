/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationImpl.h - Esp8266 WiFi Station
 *
 ****/

#pragma once

#include <Platform/Station.h>
#include <Platform/System.h>
#include <esp_wifi.h>

#ifdef ENABLE_SMART_CONFIG
#include <esp_smartconfig.h>
#endif

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
	IPAddress getIP() const override;
	MACAddress getMacAddr() const override;
	IPAddress getNetworkMask() const override;
	IPAddress getNetworkGateway() const override;
	IPAddress getNetworkBroadcast() const override;
	bool setIP(IPAddress address, IPAddress netmask, IPAddress gateway) override;
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

protected:
	void onSystemReady() override;

private:
	static void staticScanCompleted(void* arg, STATUS status);
#ifdef ENABLE_SMART_CONFIG
	void internalSmartConfig(sc_status status, void* pdata);
#endif
#ifdef ENABLE_WPS
	void internalWpsConfig(wps_cb_status status);
#endif

private:
	bool runScan = false;
#ifdef ENABLE_SMART_CONFIG
	SmartConfigEventInfo* smartConfigEventInfo = nullptr; ///< Set during smart handling
#endif
};
