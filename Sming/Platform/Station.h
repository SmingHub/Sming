/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * Station.h
 *
 ****/

/**	@defgroup wifi_sta WiFi Station Interface
 *	@brief	Control and monitoring of WiFi station interface
 *	@note   The WiFi station interface provides client access to a WiFi network.
            Control of WiFi connection including WiFi SSID and password and
            IP address, DHCP, etc.
 *  @see    \ref wifi_ap
*/

#pragma once

#include <WString.h>
#include <WVector.h>
#include <IPAddress.h>
#include <MACAddress.h>
#include "BssInfo.h"

/** @ingroup constants
 *  @{
 */
/// WiFi station connection states
enum StationConnectionStatus {
	eSCS_Idle,				  ///< Connection idle
	eSCS_Connecting,		  ///< Connecting
	eSCS_WrongPassword,		  ///< Wrong password
	eSCS_AccessPointNotFound, ///< AP not found
	eSCS_ConnectionFailed,	///< Connection failed
	eSCS_GotIP				  ///< Got IP address
};

/// Smart configuration type
enum SmartConfigType {
	SCT_None = -1,
	SCT_EspTouch,		  ///< ESP Touch
	SCT_AirKiss,		  ///< Air Kiss
	SCT_EspTouch_AirKiss, ///< ESP Touch and Air Kiss
};

/// Smart configuration event
enum SmartConfigEvent {
	SCE_Wait,		 ///< Wait
	SCE_FindChannel, ///< Find channel
	SCE_GettingSsid, ///< Getting SSID & password
	SCE_Link,		 ///< Link established
	SCE_LinkOver,	///< Link-over
};

/// Smart Config callback information
struct SmartConfigEventInfo {
	SmartConfigType type = SCT_None; ///< Type of configuration underway
	String ssid;					 ///< AP SSID
	String password;				 ///< AP Password
	bool bssidSet = false;			 ///< true if connection should match both SSID and BSSID
	MACAddress bssid;				 ///< AP BSSID
};

/// WiFi WPS callback status
enum WpsStatus {
	eWPS_Success = 0,
	eWPS_Failed,
	eWPS_Timeout,
	eWPS_WEP,
};

/** @} */

/** @ingroup event_handlers
 *  @{
 */

/**
 * @brief Scan complete handler function
 */
typedef Delegate<void(bool success, BssList& list)> ScanCompletedDelegate;

/**
 * @brief Smart configuration handler function
 * @param event
 * @param info
 * @retval bool return true to perform default configuration
 */
typedef Delegate<bool(SmartConfigEvent event, const SmartConfigEventInfo& info)> SmartConfigDelegate;

/**
 * @brief WPS configuration callback function
 * @param status
 * @retval bool return true to perform default configuration
 */
typedef Delegate<bool(WpsStatus status)> WPSConfigDelegate;

/** @} */

/** @brief  WiFi station class
 *  @addtogroup wifi_sta
 *  @{
 */
class StationClass
{
public:
	/**	@brief	Enable / disable WiFi station
	 *	@note	Disabling WiFi station will also disable and clear the handler set with <i>waitConnection</i>.
	 *	@param	enabled True to enable station. False to disable.
	 *	@param	save True to save operational mode to flash, False to set current operational mode only
     */
	virtual void enable(bool enabled, bool save = false) = 0;

	/**	@brief	Get WiFi station enable status
	 *	@retval	bool True if WiFi station enabled
	 */
	virtual bool isEnabled() const = 0;

	/**	@brief	Configure WiFi station
	 *	@param	ssid WiFi SSID
	 *	@param	password WiFi password
	 *	@param	autoConnectOnStartup True to auto connect. False for manual. (Default: True)
	 *	@param  save True to save the SSID and password in Flash. False otherwise. (Default: True)
	 */
	virtual bool config(const String& ssid, const String& password, bool autoConnectOnStartup = true,
						bool save = true) = 0;

	/**	@brief	Connect WiFi station to network
	 */
	virtual bool connect() = 0;

	/**	@brief	Disconnect WiFi station from network
	 */
	virtual bool disconnect() = 0;

	/**	@brief	Get WiFi station connectoin status
	 *	@retval	bool True if connected.
	 */
	bool isConnected() const;

	/**	@brief Get WiFi station connection failure status
	 *	@retval	bool True if connection failed
	 */
	bool isConnectionFailed() const;

	/**	@brief  Get WiFi station connection status
	 *	@retval	StationConnectionStatus Connection status structure
	 */
	virtual StationConnectionStatus getConnectionStatus() const = 0;

	/**	@brief	Get WiFi station connection status name
	 *	@retval	String String representing connection status
	 */
	String getConnectionStatusName() const;

	/**	@brief	Get WiFi station DHCP enabled status
	 *	@retval	bool True if DHCP enabled
	 */
	virtual bool isEnabledDHCP() const = 0;

	/**	@brief	Enable or disable WiFi station DHCP
	 *	@param	enable True to enable WiFi station DHCP
	 */
	virtual void enableDHCP(bool enable) = 0;

	/**	@brief	Set WiFi station DHCP hostname
	 *	@param	hostname - WiFi station DHCP hostname
	 */
	virtual void setHostname(const String& hostname) = 0;

	/**	@brief	Set WiFi station DHCP hostname
	 *	@retval WiFi station DHCP hostname
	 */
	virtual String getHostname() const = 0;

	/**	@brief	Get WiFi station IP address
	 *	@retval	IPAddress IP address of WiFi station
	 */
	virtual IPAddress getIP() const = 0;

	/**	@brief	Get WiFi station MAC address
	 *	@retval	MACAddress
	 */
	virtual MACAddress getMacAddr() const = 0;

	/**	@brief	Get WiFi station MAC address
	 *  @param sep Optional separator between bytes (e.g. ':')
	 *	@retval	String WiFi station MAC address
	 */
	String getMAC(char sep = '\0') const;

	/**	@brief	Get WiFi station network mask
	 *	@retval	IPAddress WiFi station network mask
	 */
	virtual IPAddress getNetworkMask() const = 0;

	/**	@brief	Get WiFi station default gateway
	 *	@retval	IPAddress WiFi station default gateway
	 */
	virtual IPAddress getNetworkGateway() const = 0;

	/**	@brief	GetWiFi station broadcast address
	 *	@retval	IPAddress WiFi statoin broadcast address
	 */
	virtual IPAddress getNetworkBroadcast() const = 0;

	/**	@brief	Set WiFi station IP address
	 *	@param	address IP address
	 *	@retval	bool True on success
	 */
	bool setIP(IPAddress address);

	/**	@brief	Set WiFi station IP parameters
	 *	@param	address IP address
	 *	@param	netmask Network mask
	 *	@param	gateway Default gatway
	 *	@retval	bool True on success
	 */
	virtual bool setIP(IPAddress address, IPAddress netmask, IPAddress gateway) = 0;

	/**	@brief	Get WiFi station SSID
	 *	@retval	String WiFi station SSID
	 */
	virtual String getSSID() const = 0;

	/**	@brief	Get WiFi station password
	 *	@retval	String WiFi station password
	 */
	virtual String getPassword() const = 0;

	/**	@brief	Get WiFi signal strength
	 *	@retval	int8_t Value in dBm
	 */
	virtual int8_t getRssi() const = 0;

	/**	@brief	Get active WiFi channel
	 *	@retval	uint8_t channel number
	 */
	virtual uint8_t getChannel() const = 0;

	/**	@brief	Start WiFi station network scan
	 *	@param	scanCompleted Function to call when scan completes
	 *	@retval	bool True on success
	 */
	virtual bool startScan(ScanCompletedDelegate scanCompleted) = 0;

#ifdef ENABLE_SMART_CONFIG
	/**	@brief	Start WiFi station smart configuration
	 *	@param	sctype Smart configuration type
	 *	@param	callback Function to call on WiFi staton smart configuration complete (Default: none)
	 *	@retval true if request successfully started, false on failure
	 *	@note If operation already in progress call will fail
	 */
	virtual bool smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback = nullptr) = 0;

	/**	@brief	Stop WiFi station smart configuration
	 */
	virtual void smartConfigStop() = 0;
#endif

#ifdef ENABLE_WPS
	/**	@brief	Start WiFi station by WPS method
	 *	@param	callback Function to call on WiFi WPS Events (Default: none)
	 */
	virtual bool wpsConfigStart(WPSConfigDelegate callback = nullptr) = 0;

	/**	@brief	Start WiFi station by WPS method
	 * 	@deprecated Use `wpsConfigStart()`
	 */
	bool beginWPSConfig() SMING_DEPRECATED
	{
		return wpsConfigStart(nullptr);
	}

	/**	@brief	Stop WiFi station WPS configuration
	 */
	virtual void wpsConfigStop() = 0;
#endif

protected:
	ScanCompletedDelegate scanCompletedCallback = nullptr;
#ifdef ENABLE_SMART_CONFIG
	SmartConfigDelegate smartConfigCallback = nullptr;
#endif
#ifdef ENABLE_WPS
	WPSConfigDelegate wpsConfigCallback = nullptr;
#endif
};

/**	@brief	Global instance of WiFi station object
 *	@note	Use WifiStation.<i>function</i> to access WiFi station functions
 *	@note	Example:
 *  @code   if(WifiStation.config("My_WiFi", "My_Password"))
                WifiStation.enable(true);
	@endcode
 */
extern StationClass& WifiStation;

/** @} */
