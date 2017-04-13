/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/**	@defgroup wifi_ap WiFi Access Point
 *	@brief	Control and monitoring of WiFi access point interface
 *	@note   The WiFi access point interface provides a WiFi network access point.
            Control of WiFi AP including WiFi SSID and password and
            IP address.
 *  @see    \ref wifi_sta
 *  @todo   How is wifi access point dhcp controlled?
*/

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
    /** @brief  Access point class
     *  @addtogroup wifi_ap
     *  @{
     */
	AccessPointClass();
	virtual ~AccessPointClass() {}

    /** @brief  Enable or disable WiFi AP
     *  @param  enabled True to enable AP. False to disable.
     *  @param	save True to save operational mode to flash, False to set current operational mode only
     */
	void enable(bool enabled, bool save = false);

    /** @brief  Get WiFi AP enable status
     *  @retval bool True if WiFi AP enabled.
     */
	bool isEnabled();

    /** @brief  Configure WiFi AP
     *  @param  ssid WiFi AP SSID
     *  @param  password WiFi AP password
     *  @param  mode WiFi AP mode
     *  @param  hidden True to hide WiFi AP (Default: Visible)
     *  @param  channel WiFi AP channel (Default: 7)
     *  @param  beaconInterval WiFi AP beacon interval in milliseconds (Default: 200ms)
     *  @retval bool True on success
     */
	bool config(String ssid, String password, AUTH_MODE mode, bool hidden = false, int channel = 7, int beaconInterval = 200);

    /** @brief  Configure WiFi AP
     *  @param  ssid WiFi AP SSID
     *  @param  password WiFi AP password
     *  @param  mode WiFi AP mode
     *  @param  hidden True to hide WiFi AP (Default: Visible)
     *  @param  channel WiFi AP channel (Default: 7)
     *  @param  beaconInterval WiFi AP beacon interval in milliseconds (Default: 200ms)
     *  @retval bool True on success
     */
	bool config(const char * ssid, const char * password, AUTH_MODE mode, bool hidden = false, int channel = 7, int beaconInterval = 200);

    /** @brief  Get WiFi AP IP address
     *  @retval IPAddress WiFi AP IP address
     */
	IPAddress getIP();

    /** @brief  Set WiFi AP IP addres
     *  @param  address New IP address for WiFi AP
     *  @retval bool True on success
     */
	bool setIP(IPAddress address);

    /** @brief  Get WiFi AP MAC address
     *  @retval String WiFi AP MAC address
     */
	String getMAC();

    /** @brief  Get WiFi AP MAC address
		 *	@param	s pointer to char buffer to fill with MAC address string
		 *	@param	bufSize buffer size
     *  @retval char * pointer to buffer holding WiFi AP MAC address
     */
	char * getMAC(char * s, size_t bufSize);

    /** @brief  Get WiFi AP MAC address
		 *	@param	mac array of 6 bytes for MAC address parts
     *  @retval uint8 * pointer to buffer holding MAC address parts
     */
	uint8 * getMAC(uint8 mac[6]);

    /** @brief  Get WiFi AP network mask
     *  @retval IPAddress WiFi AP network mask
     */
	IPAddress getNetworkMask();

    /** @brief  Get WiFi AP default gateway
     *  @retval IPAddress WiFi AP default gateway
     */
	IPAddress getNetworkGateway();

    /** @brief  Get WiFi AP broadcast address
     *  @retval IPAddress WiFi AP broadcast address
     */
	IPAddress getNetworkBroadcast();

	/**	@brief	Get WiFi access point SSID
	 *	@retval	String WiFi access point SSID
	 */
	String getSSID();

	/**	@brief	Get WiFi access point SSID
	 *	@param	s char buffer to fill with SSID string
	 *	@param	bufSize buffer size
	 *	@retval	char * pointer to buffer holding SSID string
	 */
	char * getSSID(char * s, size_t bufSize);

	/**	@brief	Get WiFi access point password
	 *	@retval	String WiFi access point password
	 */
	String getPassword();

	/**	@brief	Get WiFi access point password
	 *	@param	s char buffer to fill with SSID string
	 *	@param	bufSize buffer size
	 *	@retval	char * pointer to buffer holding WiFi access point password
	 */
	char * getPassword(char * s, size_t bufSize);
    /** @} */

protected:
	virtual void onSystemReady();

private:
	softap_config* runConfig;
};

/**	@brief	Global instance of WiFi access point object
 *	@note	Use WiFiAccessPoint.<i>function</i> to access WiFi access point functions
 *	@note	Example:
 *	@code	if(WiFiAccessPoint.config("ESP_AP", AUTH_OPEN))
 *              WiFiAccessPoint.enable(true);
 *	@endcode
 *  @ingroup wifi_ap
 */
extern AccessPointClass WifiAccessPoint;

#endif /* SMINGCORE_PLATFORM_ACCESSPOINT_H_ */
