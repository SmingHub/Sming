/*
 * WifiEvents.h
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 */

#ifndef SMINGCORE_PLATFORM_WIFIEVENTS_H_
#define SMINGCORE_PLATFORM_WIFIEVENTS_H_

#include "Delegate.h"
#include "WString.h"
#include "IPAddress.h"

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
#endif /* SMINGCORE_PLATFORM_WIFIEVENTS_H_ */
