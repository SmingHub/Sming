/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * BssInfo.h - WiFi Basic Service Set Information
 *
 ****/

#pragma once

#include <WString.h>
#include <WVector.h>
#include <MACAddress.h>

#ifdef ARCH_HOST
enum WifiAuthMode {
	AUTH_OPEN = 0,
	AUTH_WEP,
	AUTH_WPA_PSK,
	AUTH_WPA2_PSK,
	AUTH_WPA_WPA2_PSK,
	AUTH_MAX,
};
#else
typedef AUTH_MODE WifiAuthMode;
#endif

class BssInfo
{
public:
	/**	@brief	Get BSS open status
	 *	@retval	bool True if BSS open
	*/
	bool isOpen() const
	{
		return authorization == AUTH_OPEN;
	}

	/**	@brief	Get BSS authorisation method name
	 *	@retval	String
	*/
	String getAuthorizationMethodName() const;

	/**	@brief	Get BSS hash ID
	 *	@retval	uint32_t BSS hash ID
	*/
	uint32_t getHashId() const
	{
		return bssid.getHash();
	}

public:
	String ssid;				///< SSID
	MACAddress bssid;			///< BSS ID
	WifiAuthMode authorization; ///< Authorisation mode
	uint8_t channel;			///< Channel number
	int16_t rssi;				///< RSSI level
	bool hidden;				///< True if AP is hidden
};

typedef Vector<BssInfo> BssList; ///< List of BSS
