/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_PLATFORM_ACCESSPOINT_H_
#define SMINGCORE_PLATFORM_ACCESSPOINT_H_

#include <user_config.h>
#include "System.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../../Wiring/IPAddress.h"

class AccessPointClass : protected ISystemReadyHandler
{
public:
	AccessPointClass();
	virtual ~AccessPointClass() {}

	void enable(bool enabled);
	bool isEnabled();

	bool config(String ssid, String password, AUTH_MODE mode, bool hidden = false, int channel = 7, int beaconInterval = 200);

	IPAddress getIP();
	bool setIP(IPAddress address);
	String getMAC();

	IPAddress getNetworkMask();
	IPAddress getNetworkGateway();
	IPAddress getNetworkBroadcast();

protected:
	virtual void onSystemReady();

private:
	softap_config* runConfig;
};

extern AccessPointClass WifiAccessPoint;

#endif /* SMINGCORE_PLATFORM_ACCESSPOINT_H_ */
