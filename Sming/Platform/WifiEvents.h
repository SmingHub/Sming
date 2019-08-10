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

/**	@defgroup wifi_ev WiFi Events Interface
 *  @ingroup wifi
 *	@brief	Event callback interface for WiFi events
 *  @see    \ref wifi_sta
 *  @see    \ref wifi_ap
*/

#pragma once

#include <WString.h>
#include <IpAddress.h>
#include <MacAddress.h>
#include <Delegate.h>
#include "BssInfo.h"

/** @ingroup constants
 *  @{
 */

/**
 * @brief Common set of reason codes to IEEE 802.11-2007
 *
 * @note Codes at 200+ are non-standard, defined by Espressif.
 *
 * @note Some acronymns used here - see the full standard for more precise definitions.
 *	- SSID: Service Set Identifier (the visible name given to an Access Point)
 *	- BSSID: Basic Service Set Identifier (a MAC address physically identifying the AP)
 *	- IE: Information Element (standard piece of information carried within WiFi packets)
 *	- STA: Station (any device which supports WiFi, including APs though the term commonly refers to a client)
 *	- AP: Access Point (device to which other stations may be associated)
 *	- RSN: Robust Security Network
 *	- AUTH: Authentication (how a station proves its identity to another)
 *
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

/**
 * @brief Reason codes for WiFi station disconnection
 * @see WIFI_DISCONNECT_REASON_CODES_MAP
 */
enum WifiDisconnectReason {
#define XX(tag, code, desc) WIFI_DISCONNECT_REASON_##tag = code,
	WIFI_DISCONNECT_REASON_CODES_MAP(XX)
#undef XX
};

/** @} */

/** @ingroup event_handlers
 *  @{
 */

/**
 * @brief Delegate type for 'station connected' event
 * @param ssid
 * @param bssid
 * @param channel
 * @note This event occurs when the station successfully connects to the target AP. Upon receiving this event,
 * the DHCP client begins the process of getting an IP address.
 */
typedef Delegate<void(const String& ssid, MacAddress bssid, uint8_t channel)> StationConnectDelegate;

/**
 * @brief Delegate type for 'station disconnected' event
 * @param ssid SSID from which we've disconnected
 * @param bssid
 * @param reason Why the connection was dropped
 * @note This event can be generated in the following scenarios:
 * 	- When the station is already connected to the AP, and a manual disconnect or re-configuration is taking place. e.g. `WifiStation.disconnect()`
 * 	- When `WifiStation.connect()` is called, but the Wi-Fi driver fails to set up a connection with the AP due to certain reasons,
 * 	  e.g. the scan fails to find the target AP, authentication times out, etc. If there are more than one AP with the same SSID,
 * 	  the disconnected event is raised after the station fails to connect all of the found APs.
 * 	- When the Wi-Fi connection is disrupted because of specific reasons, e.g., the station continuously loses N beacons, the AP kicks off the station,
 * 	  the AP's authentication mode is changed, etc.
 */
typedef Delegate<void(const String& ssid, MacAddress bssid, WifiDisconnectReason reason)> StationDisconnectDelegate;

/**
 * @brief Delegate type for 'station authorisation mode changed' event
 * @param oldMode
 * @param newMode
 * @note This event arises when the AP to which the station is connected changes its authentication mode,
 * e.g., from 'no auth' to WPA. Generally, the application event callback does not need to handle this.
 */
typedef Delegate<void(WifiAuthMode oldMode, WifiAuthMode newMode)> StationAuthModeChangeDelegate;

/**
 * @brief Delegate type for 'station got IP address' event
 * @param ip
 * @param netmask
 * @param gateway
 * @note This event arises when the DHCP client successfully gets the IPV4 address from the DHCP server,
 * or when the IPV4 address is changed. The IPV4 may be changed because of the following reasons:
 *	- The DHCP client fails to renew/rebind the IPV4 address, and the station's IPV4 is reset to 0.
 *	- The DHCP client rebinds to a different address.
 *	- The static-configured IPV4 address is changed.
 */
typedef Delegate<void(IpAddress ip, IpAddress netmask, IpAddress gateway)> StationGotIPDelegate;

/**
 * @brief Delegate type for 'Access Point Connect' event
 * @param mac MAC address of the station
 * @param aid Association ID representing the connected station
 * @note This event occurs every time a station is connected to our Access Point.
 */
typedef Delegate<void(MacAddress mac, uint16_t aid)> AccessPointConnectDelegate;

/**
 * @brief Delegate type for 'Access Point Disconnect' event
 * @param mac MAC address of the station
 * @param aid Association ID assigned to the station
 * @note This event occurs every time a station is disconnected from our Access Point.
 */
typedef Delegate<void(MacAddress mac, uint16_t aid)> AccessPointDisconnectDelegate;

/**
 * @brief Delegate type for 'Access Point Probe Request Received' event
 * @param rssi Signal strength
 * @param mac
 * @note Probe Requests are a low-level management frame which are used to determine
 * informaton about our Access Point, such as which authentication modes are supported.
 */
typedef Delegate<void(int rssi, MacAddress mac)> AccessPointProbeReqRecvedDelegate;

/** @} */

/** @brief  WiFi events class
 *  @addtogroup wifi_ev
 *  @{
 */
class WifiEventsClass
{
public:
	/**
	 * @brief Set callback for 'station connected' event
	 */
	void onStationConnect(StationConnectDelegate delegateFunction)
	{
		onSTAConnect = delegateFunction;
	}

	/**
	 * @brief Set callback for 'station disconnected' event
	 */
	void onStationDisconnect(StationDisconnectDelegate delegateFunction)
	{
		onSTADisconnect = delegateFunction;
	}

	/**
	 * @brief Get short name for disconnection reason
	 */
	static String getDisconnectReasonName(WifiDisconnectReason reason);

	/**
	 * @brief Get descriptive explanation for disconnect reason
	 */
	static String getDisconnectReasonDesc(WifiDisconnectReason reason);

	/**
	 * @brief Set callback for 'station authorisation mode change' event
	 */
	void onStationAuthModeChange(StationAuthModeChangeDelegate delegateFunction)
	{
		onSTAAuthModeChange = delegateFunction;
	}

	/**
	 * @brief Set callback for 'station connected with IP address' event
	 */
	void onStationGotIP(StationGotIPDelegate delegateFunction)
	{
		onSTAGotIP = delegateFunction;
	}

	/**
	 * @brief Set callback for 'access point client connected' event
	 */
	void onAccessPointConnect(AccessPointConnectDelegate delegateFunction)
	{
		onSOFTAPConnect = delegateFunction;
	}

	/**
	 * @brief Set callback for 'access point client disconnected' event
	 */
	void onAccessPointDisconnect(AccessPointDisconnectDelegate delegateFunction)
	{
		onSOFTAPDisconnect = delegateFunction;
	}

	/**
	 * @brief Set callback for 'access point probe request received' event
	 */
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

/** @} */
