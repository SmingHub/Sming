/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 ****/

/**	@defgroup wifi_sta WiFi Station Interface
 *	@brief	Control and monitoring of WiFi station interface
 *	@note   The WiFi station interface provides client access to a WiFi network.
            Control of WiFi connection including WiFi SSID and password and
            IP address, DHCP, etc.
 *  @see    \ref wifi_ap
*/

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

/** @ingroup constants
 *  @{
 */
/// WiFi station connection states
enum EStationConnectionStatus
{
	eSCS_Idle = 0, ///< WiFi station connection idle
	eSCS_Connecting, ///< Wifi station connecting
	eSCS_WrongPassword, ///< WiFi station wrong password
	eSCS_AccessPointNotFound, ///< WiFi station AP not found
	eSCS_ConnectionFailed,  ///< WiFi station connectoin failed
	eSCS_GotIP ///< WiFi station got IP address
};

/// Smart configuration type
enum SmartConfigType
{
	SCT_EspTouch = SC_TYPE_ESPTOUCH, ///< WiFi station smart configuration ESP touch
	SCT_AirKiss = SC_TYPE_AIRKISS, ///< WiFi station smart configuration Air Kiss
	SCT_EspTouch_AirKiss = SC_TYPE_ESPTOUCH_AIRKISS ///< WiFi station smart configuration ESP Touch and Air Kiss
};

/// Smart configuration event
enum SmartConfigEvent
{
	SCE_Wait = SC_STATUS_WAIT, ///< Smart configuration wait state
	SCE_FindChannel = SC_STATUS_FIND_CHANNEL, ///< Smart configuration find channel state
	SCE_GotSsid = SC_STATUS_GETTING_SSID_PSWD, ///< Smart configuration getting SSID & password state
	SEC_Link = SC_STATUS_LINK, ///< Smart configuration link established state
	SEC_LinkOver = SC_STATUS_LINK_OVER ///< Smart configuration link over status
};

class BssInfo;
class Timer;

typedef Vector<BssInfo> BssList; ///< List of BSS
/** @} */

/** @ingroup event_handlers
 *  @{
 */
typedef Delegate<void(bool, BssList)> ScanCompletedDelegate; ///< Scan complete handler function
typedef Delegate<void()> ConnectionDelegate; ///< Connection handler function
typedef Delegate<void(sc_status status, void *pdata)> SmartConfigDelegate; ///< Smart configuration handler function
/** @} */

class StationClass : protected ISystemReadyHandler
{
public:
    /** @brief  WiFi station class
     *  @addtogroup wifi_sta
     *  @{
     */
	StationClass();
	~StationClass();

	/**	@brief	Enable / disable WiFi station
	 *	@note	Disabling WiFi station will also disable and clear the handler set with <i>waitConnection</i>.
	 *	@param	enabled True to enable station. False to disable.
	 *	@param	save True to save operational mode to flash, False to set current operational mode only
     */
	void enable(bool enabled, bool save = false);

	/**	@brief	Get WiFi station enable status
	 *	@retval	bool True if WiFi station enabled
	 */
	bool isEnabled();

	/**	@brief	Configure WiFi station
	 *	@param	ssid WiFi SSID
	 *	@param	password WiFi password
	 *	@param	autoConnectOnStartup True to auto connect. False for manual. (Default: True)
	 *	@param  save True to save the SSID and password in Flash. False otherwise. (Default: True)
	 */
	bool config(const char * ssid, const char * password, bool autoConnectOnStartup = true, bool save = true);

	/**	@brief	Configure WiFi station
	 *	@param	ssid WiFi SSID
	 *	@param	password WiFi password
	 *	@param	autoConnectOnStartup True to auto connect. False for manual. (Default: True)
	 *	@param  save True to save the SSID and password in Flash. False otherwise. (Default: True)
	 */
	bool config(String ssid, String password, bool autoConnectOnStartup = true, bool save = true);

	/**	@brief	Connect WiFi station to network
	 */
	bool connect();

	/**	@brief	Disconnect WiFi station from network
	 */
	bool disconnect();

	/**	@brief	Get WiFi station connectoin status
	 *	@retval	bool True if connected.
	 */
	bool isConnected();

	/**	@brief Get WiFi station connection failure status
	 *	@retval	bool True if connection failed
	 */
	bool isConnectionFailed();

	/**	@brief  Get WiFi station connection status
	 *	@retval	EStationConnectionStatus Connection status structure
	 */
	EStationConnectionStatus getConnectionStatus();

	/**	@brief	Get WiFi station connection status name
	 *	@retval	char* Pointer to c string name of connection status
	 */
	const char* getConnectionStatusName();

	/**	@brief	Get WiFi station DHCP enabled status
	 *	@retval	bool True if DHCP enabled
	 */
	bool isEnabledDHCP();

	/**	@brief	Enable or disable WiFi station DHCP
	 *	@param	enable True to enable WiFi station DHCP
	 */
	void enableDHCP(bool enable);

	/**	@brief	Set WiFi station DHCP hostname
	 *	@param	hostname - WiFi station DHCP hostname
	 */
	void setHostname(String hostname);

	/**	@brief	Set WiFi station DHCP hostname
	 *	@param	hostname - WiFi station DHCP hostname
	 */
	void setHostname(char * hostname);

	/**	@brief	Get WiFi station hostname
	 *	@retval WiFi station hostname as String
	 */
	String getHostname();

	/**	@brief	Get WiFi station hostname
	 *  @param  s buffer to be filled with hostname
	 *  @param  bufSize buffer size
	 *	@retval char* pointer to buffer holding hostname
	 */
	char * getHostname(char * s, size_t bufSize);

	/**	@brief	Get WiFi station IP address
	 *	@retval	IPAddress IP address of WiFi station
	 */
	IPAddress getIP();

	/**	@brief	Get WiFi station MAC address
	 *	@retval	String WiFi station MAC address
	 */
	String getMAC();

	/**	@brief	Get WiFi station MAC address
	 *  @param  s buffer to be filled with MAC address
	 *  @param  bufSize buffer size
	 *	@retval	char* pointer to buffer holding MAC address
	 */
	char * getMAC(char * s, size_t bufSize);

	/**	@brief	Get WiFi station MAC address
	 *  @param  mac 6 byte array to fill with MAC address bytes
	 *	@retval	uint8* pointer to array holding MAC address bytes
	 */
	uint8 * getMAC(uint8 mac[6]);

	/**	@brief	Get WiFi station network mask
	 *	@retval	IPAddress WiFi station network mask
	 */
	IPAddress getNetworkMask();

	/**	@brief	Get WiFi station default gateway
	 *	@retval	IPAddress WiFi station default gateway
	 */
	IPAddress getNetworkGateway();

	/**	@brief	GetWiFi station broadcast address
	 *	@retval	IPAddress WiFi statoin broadcast address
	 */
	IPAddress getNetworkBroadcast();

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
	bool setIP(IPAddress address, IPAddress netmask, IPAddress gateway);

	/**	@brief	Get WiFi station SSID
	 *	@retval	String WiFi station SSID
	 */
	String getSSID();

	/**	@brief	Get WiFi station SSID
	 *  @param s buffer to fill with SSID
	 *  @param bufSize buffer size
	 *	@retval	char* pointer to buffer holding SSID
	 */
	char * getSSID(char * s, size_t bufSize);

	/**	@brief	Get WiFi station password
	 *	@retval	String WiFi station password
	 */
	String getPassword();

	/**	@brief	Get WiFi station password
	 *  @param s buffer to fill with password
	 *  @param bufSize buffer size
	 *	@retval	char* pointer to buffer holding WiFi station password
	 */
	char * getPassword(char * s, size_t bufSize);

	sint8 getRssi();
	uint8 getChannel(); 

	/**	@brief	Start WiFi station network scan
	 *	@param	scanCompleted Function to call when scan completes
	 *	@retval	bool True on success
	 */
	bool startScan(ScanCompletedDelegate scanCompleted);

	/**	@brief	Assign handler for WiFi station connection
	 *	@note	The handler will be cleared if the WiFi Station is disabled. If you subsequently reenable WiFi Station, another call to <i>waitConnection</i> must be made if you want the handler to be reinstalled.
	 *	@param	successfulConnected Function to call when WiFi station connects to network
	 */
	void waitConnection(ConnectionDelegate successfulConnected);

	/**	@brief	Assign handler for WiFi station connection with timeout
	 *	@note	The handler will be cleared if the WiFi Station is disabled. If you subsequently reenable WiFi Station, another call to <i>waitConnection</i> must be made if you want the handler to be reinstalled.
	 *	@param	successfulConnected Function to call when WiFi station connects to network
	 *	@param	secondsTimeOut Quantity of seconds to wait for connection
	 *	@param	connectionNotEstablished Function to call if WiFi station fails to connect to network
	 *
	 *	@deprecated This method is deprecated and will be removed in the next versions. Use WifiEvents instead.
	 *				For an example of WifiEvents take a look at the Basic_Wifi sample code.
	 *
	 */
	void waitConnection(ConnectionDelegate successfulConnected, int secondsTimeOut, ConnectionDelegate connectionNotEstablished);

	/**	@brief	Start WiFi station smart configuration
	 *	@param	sctype Smart configuration type
	 *	@param	callback Function to call on WiFi staton smart configuration complete (Default: none)
	 */
	void smartConfigStart(SmartConfigType sctype, SmartConfigDelegate callback = NULL);

	/**	@brief	Stop WiFi station smart configuration
	 */
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

	/**	@brief	Get BSS open status
	 *	@retval	bool True if BSS open
	*/
	bool isOpen();

	/**	@brief	Get BSS authorisation method name
	 *	@retval	char* Pointer to c string BSS authoristation method name
	*/
	const char* getAuthorizationMethodName();

	/**	@brief	Get BSS hash ID
	 *	@retval	uint32_t BSS hash ID
	*/
	uint32_t getHashId();

public:
	String ssid; ///< SSID
	uint8 bssid[6]; ///< BSS ID
	AUTH_MODE authorization; ///< Authorisation mode
	uint8 channel; ///< Channel number
	sint16 rssi; ///< RSSI level
	bool hidden; ///< True if AP is hidden
};

/**	@brief	Global instance of WiFi station object
 *	@note	Use WiFiStation.<i>function</i> to access WiFi station functions
 *	@note	Example:
 *  @code   if(WiFiStation.config("My_WiFi", "My_Password"))
                WiFiStation.enable(true);
	@endcode
 */
extern StationClass WifiStation;

/** @} */
#endif /* SMINGCORE_PLATFORM_STATION_H_ */
