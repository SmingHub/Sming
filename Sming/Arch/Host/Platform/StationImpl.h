/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * StationImpl.h - Host WiFi Station
 *
 ****/

#pragma once

#include <Platform/Station.h>

struct netif;

class StationImpl : public StationClass
{
public:
	struct ApInfo {
		const char* ssid;
		MACAddress bssid;
		WifiAuthMode authMode;
		uint8_t channel;
		bool hidden;
		const char* pwd;
	};

	StationImpl();
	void initialise(netif* nif);
	const ApInfo* getApInfo()
	{
		return currentAp;
	}

	// StationClass
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

private:
	void statusCallback(netif* nif);

private:
	ApInfo* currentAp;
	ApInfo* savedAp;

	struct StationConfig {
		bool enabled = true;
	};
	StationConfig currentConfig;
	StationConfig savedConfig;
	bool autoConnect = true;
	bool connected = true;
	String hostName;
	StationConnectionStatus connectionStatus = eSCS_Idle;
	bool dhcpEnabled = true;
};
