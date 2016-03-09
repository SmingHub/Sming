/*
 * WifiEvents.h
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 */

#ifndef SMINGCORE_PLATFORM_WIFIEVENTS_H_
#define SMINGCORE_PLATFORM_WIFIEVENTS_H_

#include "../SmingCore/Delegate.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/IPAddress.h"

//Define WifiEvents Delegates types
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> onStationConnectDelegate;
typedef Delegate<void(String, uint8_t, uint8_t[6], uint8_t)> onStationDisconnectDelegate;
typedef Delegate<void(uint8_t, uint8_t)> onStationAuthModeChangeDelegate;
typedef Delegate<void(IPAddress, IPAddress, IPAddress)> onStationGotIPDelegate;
typedef Delegate<void(uint8_t[6], uint8_t)> onAccessPointConnectDelegate;
typedef Delegate<void(uint8_t[6], uint8_t)> onAccessPointDisconnectDelegate;
typedef Delegate<void(int16_t, uint8_t[6])> onAccessPointProbeReqRecvedDelegate;

class WifiEventsClass
{
public:
	WifiEventsClass();

	void onStationConnect(onStationConnectDelegate delegateFunction);
	void onStationDisconnect(onStationDisconnectDelegate delegateFunction);
	void onStationAuthModeChange(onStationAuthModeChangeDelegate delegateFunction);
	void onStationGotIP(onStationGotIPDelegate delegateFunction);
	void onAccessPointConnect(onAccessPointConnectDelegate delegateFunction);
	void onAccessPointDisconnect(onAccessPointDisconnectDelegate delegateFunction);
	void onAccessPointProbeReqRecved(onAccessPointProbeReqRecvedDelegate delegateFunction);

private:
	static void staticWifiEventHandler(System_Event_t *evt);
	void WifiEventHandler(System_Event_t *evt);

	onStationConnectDelegate onSTAConnect = nullptr;
	onStationDisconnectDelegate onSTADisconnect = nullptr;
	onStationAuthModeChangeDelegate onSTAAuthModeChange = nullptr;
	onStationGotIPDelegate onSTAGotIP = nullptr;
	onAccessPointConnectDelegate onSOFTAPConnect = nullptr;
	onAccessPointDisconnectDelegate onSOFTAPDisconnect = nullptr;
	onAccessPointProbeReqRecvedDelegate onSOFTAPProbeReqRecved = nullptr;
};


extern WifiEventsClass WifiEvents;
#endif /* SMINGCORE_PLATFORM_WIFIEVENTS_H_ */
