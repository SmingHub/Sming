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
#include "../../Wiring/WString.h"
#include "../../Wiring/WVector.h"
#include "../../Wiring/IPAddress.h"

enum EStationConnectionStatus
{
    eSCS_Idle = 0,
	eSCS_Connecting,
	eSCS_WrongPassword,
	eSCS_AccessPointNotFound,
	eSCS_ConnectionFailed,
	eSCS_GotIP
};

class BssInfo;
class Timer;

typedef Vector<BssInfo> BssList;
typedef void (*ScanCompletedCallback)(bool succeeded, BssList list);
typedef void (*ConnectionCallback)();

class StationClass : protected ISystemReadyHandler
{
public:
	StationClass();
	~StationClass();

	void enable(bool enabled);
	bool isEnabled();

	bool config(String ssid, String password, bool autoConnectOnStartup = true);

	bool isConnected();
	bool isConnectionFailed();
	EStationConnectionStatus getConnectionStatus();
	const char* getConnectionStatusName();

	IPAddress getIP();
	bool setIP(IPAddress address);
	bool setIP(IPAddress address, IPAddress netmask, IPAddress gateway);
	String getMAC();

	String getSSID();
	String getPassword();

	bool startScan(ScanCompletedCallback scanCompleted);
	void waitConnection(ConnectionCallback successfulConnected);
	void waitConnection(ConnectionCallback successfulConnected, int secondsTimeOut, ConnectionCallback connectionNotEstablished);

protected:
	virtual void onSystemReady();
	static void staticScanCompleted(void *arg, STATUS status);

	void internalCheckConnection();
	static void staticCheckConnection();

private:
	ScanCompletedCallback scanCompletedCallback;
	bool runScan;

	ConnectionCallback onConnectOk;
	ConnectionCallback onConnectFail;
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

public:
	String ssid;
	AUTH_MODE authorization;
	uint8 channel;
	sint16 rssi;
	bool hidden;
};

extern StationClass WifiStation;

#endif /* SMINGCORE_PLATFORM_STATION_H_ */
