/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEvents.h
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 *
 ****/

#pragma once

#include "Delegate.h"
#include "WString.h"
#include "IPAddress.h"

#include <esp_wifi.h>

//Define WifiEvents Delegates types
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> StationConnectDelegate;
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> StationDisconnectDelegate;
typedef Delegate<void(uint8_t, uint8_t)> StationAuthModeChangeDelegate;
typedef Delegate<void(IPAddress, IPAddress, IPAddress)> StationGotIPDelegate;
typedef Delegate<void(uint8_t[6], uint8_t)> AccessPointConnectDelegate;
typedef Delegate<void(uint8_t[6], uint8_t)> AccessPointDisconnectDelegate;
typedef Delegate<void(int16_t, uint8_t[6])> AccessPointProbeReqRecvedDelegate;

class WifiEventsClass
{
public:
	WifiEventsClass();

	void onStationConnect(StationConnectDelegate delegateFunction)
	{
		onSTAConnect = delegateFunction;
	}

	void onStationDisconnect(StationDisconnectDelegate delegateFunction)
	{
		onSTADisconnect = delegateFunction;
	}

	void onStationAuthModeChange(StationAuthModeChangeDelegate delegateFunction)
	{
		onSTAAuthModeChange = delegateFunction;
	}

	void onStationGotIP(StationGotIPDelegate delegateFunction)
	{
		onSTAGotIP = delegateFunction;
	}

	void onAccessPointConnect(AccessPointConnectDelegate delegateFunction)
	{
		onSOFTAPConnect = delegateFunction;
	}

	void onAccessPointDisconnect(AccessPointDisconnectDelegate delegateFunction)
	{
		onSOFTAPDisconnect = delegateFunction;
	}

	void onAccessPointProbeReqRecved(AccessPointProbeReqRecvedDelegate delegateFunction)
	{
		onSOFTAPProbeReqRecved = delegateFunction;
	}

private:
	void WifiEventHandler(System_Event_t* evt);

	StationConnectDelegate onSTAConnect = nullptr;
	StationDisconnectDelegate onSTADisconnect = nullptr;
	StationAuthModeChangeDelegate onSTAAuthModeChange = nullptr;
	StationGotIPDelegate onSTAGotIP = nullptr;
	AccessPointConnectDelegate onSOFTAPConnect = nullptr;
	AccessPointDisconnectDelegate onSOFTAPDisconnect = nullptr;
	AccessPointProbeReqRecvedDelegate onSOFTAPProbeReqRecved = nullptr;
};

extern WifiEventsClass WifiEvents;
