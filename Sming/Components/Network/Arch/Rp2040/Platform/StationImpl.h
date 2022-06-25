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

class StationImpl : public StationClass, protected ISystemReadyHandler
{
public:
	StationImpl()
	{
		System.onReady(this);
	}

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

protected:
	void onSystemReady() override;
};
