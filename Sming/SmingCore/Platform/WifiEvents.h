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

#ifndef _SMING_CORE_PLATFORM_WIFI_EVENTS_H_
#define _SMING_CORE_PLATFORM_WIFI_EVENTS_H_

#include "../SmingCore/Delegate.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/IPAddress.h"

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

	void onStationConnect(StationConnectDelegate delegateFunction);
	void onStationDisconnect(StationDisconnectDelegate delegateFunction);
	void onStationAuthModeChange(StationAuthModeChangeDelegate delegateFunction);
	void onStationGotIP(StationGotIPDelegate delegateFunction);
	void onAccessPointConnect(AccessPointConnectDelegate delegateFunction);
	void onAccessPointDisconnect(AccessPointDisconnectDelegate delegateFunction);
	void onAccessPointProbeReqRecved(AccessPointProbeReqRecvedDelegate delegateFunction);

private:
	static void staticWifiEventHandler(System_Event_t* evt);
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
#endif /* _SMING_CORE_PLATFORM_WIFI_EVENTS_H_ */
