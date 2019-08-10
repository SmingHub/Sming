/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * AccessPoint.h
 *
 ****/

/**	@defgroup wifi_ap WiFi Access Point
 *  @ingroup wifi
 *	@brief	Control and monitoring of WiFi access point interface
 *	@note   The WiFi access point interface provides a WiFi network access point.
            Control of WiFi AP including WiFi SSID and password and
            IP address.
 *  @see    \ref wifi_sta
 *  @see    \ref wifi_ev
 *  @todo   How is wifi access point dhcp controlled?
*/

#pragma once

#include <WString.h>
#include <IpAddress.h>
#include <MacAddress.h>
#include "BssInfo.h"

/** @brief  Access point class
 *  @addtogroup wifi_ap
 *  @{
 */
class AccessPointClass
{
public:
	/** @brief  Enable or disable WiFi AP
     *  @param  enabled True to enable AP. False to disable.
     *  @param	save True to save operational mode to flash, False to set current operational mode only
     */
	virtual void enable(bool enabled, bool save = false) = 0;

	/** @brief  Get WiFi AP enable status
     *  @retval bool True if WiFi AP enabled.
     */
	virtual bool isEnabled() const = 0;

	/** @brief  Configure WiFi AP
     *  @param  ssid WiFi AP SSID
     *  @param  password WiFi AP password
     *  @param  mode WiFi AP mode
     *  @param  hidden True to hide WiFi AP (Default: Visible)
     *  @param  channel WiFi AP channel (Default: 7)
     *  @param  beaconInterval WiFi AP beacon interval in milliseconds (Default: 200ms)
     *  @retval bool True on success
     */
	virtual bool config(const String& ssid, String password, WifiAuthMode mode, bool hidden = false, int channel = 7,
						int beaconInterval = 200) = 0;

	/** @brief  Get WiFi AP IP address
     *  @retval IpAddress WiFi AP IP address
     */
	virtual IpAddress getIP() const = 0;

	/** @brief  Set WiFi AP IP addres
     *  @param  address New IP address for WiFi AP
     *  @retval bool True on success
     */
	virtual bool setIP(IpAddress address) = 0;

	/**	@brief	Get WiFi AP MAC address
	 *	@retval	MacAddress
	 */
	virtual MacAddress getMacAddress() const = 0;

	/** @brief  Get WiFi AP MAC address
	 *  @param	sep separator between bytes (e.g. ':')
     *  @retval String WiFi AP MAC address
     */
	String getMAC(char sep = '\0') const;

	/** @brief  Get WiFi AP network mask
     *  @retval IpAddress WiFi AP network mask
     */
	virtual IpAddress getNetworkMask() const = 0;

	/** @brief  Get WiFi AP default gateway
     *  @retval IpAddress WiFi AP default gateway
     */
	virtual IpAddress getNetworkGateway() const = 0;

	/** @brief  Get WiFi AP broadcast address
     *  @retval IpAddress WiFi AP broadcast address
     */
	virtual IpAddress getNetworkBroadcast() const = 0;

	/**	@brief	Get WiFi access point SSID
	 *	@retval	String WiFi access point SSID
	 */
	virtual String getSSID() const = 0;

	/**	@brief	Get WiFi access point password
	 *	@retval	String WiFi access point password
	 */
	virtual String getPassword() const = 0;
};
/** @} */

/**	@brief	Global instance of WiFi access point object
 *	@note	Use WifiAccessPoint.<i>function</i> to access WiFi access point functions
 *	@note	Example:
 *	@code	if(WifiAccessPoint.config("ESP_AP", AUTH_OPEN))
 *              WifiAccessPoint.enable(true);
 *	@endcode
 *  @ingroup wifi_ap
 */
extern AccessPointClass& WifiAccessPoint;
