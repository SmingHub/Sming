/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiEvents.h
 *
 *  Created on: 19 февр. 2016 г.
 *      Author: shurik
 *
 ****/

#pragma once

#include <WString.h>
#include <IPAddress.h>
#include <MACAddress.h>
#include <Delegate.h>
#include "BssInfo.h"

/*
 * Common set of reason codes to ieee 802.11-2007
 * Codes at 200+ are non-standard defined by Espressif.
 *
 * Some acronymns:
 *
 * 	IE: Information Element
 * 	STA: Station
 * 	AP: Access Point
 *  RSN:
 *  AUTH: Authentication
 */
#define WIFI_DISCONNECT_REASON_CODES_MAP(XX)                                                                           \
	XX(UNSPECIFIED, 1, "Unspecified")                                                                                  \
	XX(AUTH_EXPIRE, 2, "AUTH expired")                                                                                 \
	XX(AUTH_LEAVE, 3, "Sending STA is leaving, or has left")                                                           \
	XX(ASSOC_EXPIRE, 4, "Disassociated: inactivity")                                                                   \
	XX(ASSOC_TOOMANY, 5, "Disassociated: too many clients)")                                                           \
	XX(NOT_AUTHED, 6, "Class 2 frame received from non-authenticated STA")                                             \
	XX(NOT_ASSOCED, 7, "Class 3 frame received from non-authenticated STA")                                            \
	XX(ASSOC_LEAVE, 8, "Disassociated: STA is leaving, or has left")                                                   \
	XX(ASSOC_NOT_AUTHED, 9, "Disassociated: STA not authenticated")                                                    \
	XX(DISASSOC_PWRCAP_BAD, 10, "Disassociated: power capability unacceptable")                                        \
	XX(DISASSOC_SUPCHAN_BAD, 11, "Disassociated: supported channels unacceptable")                                     \
	XX(IE_INVALID, 13, "Invalid IE")                                                                                   \
	XX(MIC_FAILURE, 14, "Message Integrity failure")                                                                   \
	XX(4WAY_HANDSHAKE_TIMEOUT, 15, "4-way Handshake timeout")                                                          \
	XX(GROUP_KEY_UPDATE_TIMEOUT, 16, "Group Key Handshake timeout")                                                    \
	XX(IE_IN_4WAY_DIFFERS, 17, "4-way Handshake Information Differs")                                                  \
	XX(GROUP_CIPHER_INVALID, 18, "Invalid group cypher")                                                               \
	XX(PAIRWISE_CIPHER_INVALID, 19, "Invalid pairwise cypher")                                                         \
	XX(AKMP_INVALID, 20, "Invalid AKMP")                                                                               \
	XX(UNSUPP_RSN_IE_VERSION, 21, "Unsupported RSN IE Version")                                                        \
	XX(INVALID_RSN_IE_CAP, 22, "Invalid RSN IE capabilities")                                                          \
	XX(802_1X_AUTH_FAILED, 23, "IEEE 802.1X authentication failed")                                                    \
	XX(CIPHER_SUITE_REJECTED, 24, "Cipher suite rejected (security policy)")                                           \
	XX(BEACON_TIMEOUT, 200, "Beacon Timeout")                                                                          \
	XX(NO_AP_FOUND, 201, "No AP found")                                                                                \
	XX(AUTH_FAIL, 202, "Authentication failure")                                                                       \
	XX(ASSOC_FAIL, 203, "Association failure")                                                                         \
	XX(HANDSHAKE_TIMEOUT, 204, "Handshake timeout")                                                                    \
	XX(CONNECTION_FAIL, 205, "Connection failure")

enum WifiDisconnectReason {
#define XX(tag, code, desc) WIFI_DISCONNECT_REASON_##tag = code,
	WIFI_DISCONNECT_REASON_CODES_MAP(XX)
#undef XX
};

// Define WifiEvents Delegates types
typedef Delegate<void(const String& ssid, const MACAddress& bssid, uint8_t channel)> StationConnectDelegate;
typedef Delegate<void(const String& ssid, const MACAddress& bssid, WifiDisconnectReason reason)>
	StationDisconnectDelegate;
typedef Delegate<void(WifiAuthMode oldMode, WifiAuthMode newMode)> StationAuthModeChangeDelegate;
typedef Delegate<void(IPAddress ip, IPAddress netmask, IPAddress gateway)> StationGotIPDelegate;
typedef Delegate<void(const MACAddress& mac, uint8_t aid)> AccessPointConnectDelegate;
typedef Delegate<void(const MACAddress& mac, uint8_t aid)> AccessPointDisconnectDelegate;
typedef Delegate<void(int rssi, const MACAddress& mac)> AccessPointProbeReqRecvedDelegate;

class WifiEventsClass
{
public:
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

protected:
	StationConnectDelegate onSTAConnect = nullptr;
	StationDisconnectDelegate onSTADisconnect = nullptr;
	StationAuthModeChangeDelegate onSTAAuthModeChange = nullptr;
	StationGotIPDelegate onSTAGotIP = nullptr;
	AccessPointConnectDelegate onSOFTAPConnect = nullptr;
	AccessPointDisconnectDelegate onSOFTAPDisconnect = nullptr;
	AccessPointProbeReqRecvedDelegate onSOFTAPProbeReqRecved = nullptr;
};

extern WifiEventsClass& WifiEvents;
