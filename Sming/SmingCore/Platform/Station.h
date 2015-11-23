/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

#ifndef SMINGCORE_PLATFORM_STATION_H_
#define SMINGCORE_PLATFORM_STATION_H_

#include <user_config.h>
#include "System.h"
#include "../SmingCore/Delegate.h"
#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../../Wiring/IPAddress.h"

extern "C" {
	#include <smartconfig.h>
}

enum EStationConnectionStatus
{
	eSCS_Idle = 0,
	eSCS_Connecting,
	eSCS_WrongPassword,
	eSCS_AccessPointNotFound,
	eSCS_ConnectionFailed,
	eSCS_GotIP
};

enum SmartConfigType
{
	SCT_EspTouch = SC_TYPE_ESPTOUCH,
	SCT_AirKiss = SC_TYPE_AIRKISS,
	SCT_EspTouch_AirKiss = SC_TYPE_ESPTOUCH_AIRKISS
};

enum SmartConfigEvent
{
	SCE_Wait = SC_STATUS_WAIT,
	SCE_FindChannel = SC_STATUS_FIND_CHANNEL,
	SCE_GotSsid = SC_STATUS_GETTING_SSID_PSWD,
	SEC_Link = SC_STATUS_LINK,
	SEC_LinkOver = SC_STATUS_LINK_OVER
};

class BssInfo;
class Timer;

typedef Vector<BssInfo> BssList;
typedef Delegate<void(bool, BssList)> ScanCompletedDelegate;
typedef Delegate<void()> ConnectionDelegate;
typedef Delegate<void(sc_status status, void *pdata)> SmartConfigDelegate;

class StationClass : protected ISystemReadyHandler
{
public:
	StationClass();
	~StationClass();

	void enable(bool enabled);
	bool isEnabled();

	bool config(String ssid, String password, bool autoConnectOnStartup = true);
	void disconnect();
	bool isConnected();
	bool isConnectionFailed();

	EStationConnectionStatus getConnectionStatus();
	const char* getConnectionStatusName();

	bool isEnabledDHCP();
	void enableDHCP(bool enable);

	IPAddress getIP();
	String getMAC();
	IPAddress getNetworkMask();
	IPAddress getNetworkGateway();
	IPAddress getNetworkBroadcast();

	bool setIP(IPAddress address);
	bool setIP(IPAddress address, IPAddress netmask, IPAddress gateway);

	String getSSID();
	String getPassword();

	bool startScan(ScanCompletedDelegate scanCompleted);
	void waitConnection(ConnectionDelegate successfulConnected);
	void waitConnection(ConnectionDelegate successfulConnected, int secondsTimeOut, ConnectionDelegate connectionNotEstablished);

	void smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback = NULL);
	void smartConfigStop();

protected:
	virtual void onSystemReady();
	static void staticScanCompleted(void *arg, STATUS status);

	void internalCheckConnection();
	static void staticCheckConnection();

	void internalSmartConfig(sc_status status, void *pdata);
	static void staticSmartConfigCallback(sc_status status, void *pdata);

private:
	ScanCompletedDelegate scanCompletedCallback;
	SmartConfigDelegate smartConfigCallback = NULL;
	bool runScan;

	ConnectionDelegate onConnectOk;
	ConnectionDelegate onConnectFail;
	int connectionTimeOut;
	uint32 connectionStarted;
	Timer* connectionTimer;
};

class BssInfo
{
public:
	BssInfo(bss_info* info);
	bool isOpen();
	const char* getAuthorizationMethodName();
	uint32_t getHashId();

public:
	String ssid;
	uint8 bssid[6];
	AUTH_MODE authorization;
	uint8 channel;
	sint16 rssi;
	bool hidden;
};

extern StationClass WifiStation;

#endif /* SMINGCORE_PLATFORM_STATION_H_ */
