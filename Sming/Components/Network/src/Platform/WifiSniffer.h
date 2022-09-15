/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiSniffer.h
 *
 * Original code by Ray Burnette http://www.hackster.io/rayburne/projects
 *
 * Adapted for use with Sming March 2019 mikee47 <mike@sillyhouse.net>
 *
 * See ESP8266 non-OS SDK Version 3.0, section 3.11: Sniffer Related APIs
 *
 ****/

#pragma once

#include <Platform/System.h>
#include <MacAddress.h>
#include "WVector.h"

/**	@defgroup wifi_sniffer WiFi Sniffer
 *  @ingroup wifi
 *	@brief	WiFi promiscuous mode sniffer support
 *	@{
*/

#define ETH_MAC_LEN 6

/**
 * @brief Decoded Wifi beacon (Access Point) information
 */
struct BeaconInfo {
	MacAddress bssid;
	uint8_t ssid[33];
	uint8_t ssid_len;
	uint8_t channel;
	int8_t err;
	int8_t rssi;
	uint8_t capa[2];
};

/**
 * @brief Decoded Wifi client information
 */
struct ClientInfo {
	MacAddress bssid;
	MacAddress station;
	MacAddress ap;
	uint8_t channel;
	int8_t err;
	int8_t rssi;
	uint16_t seq_n;
};

template <class T> class BeaconOrClientListTemplate : public Vector<T>
{
public:
	int indexOf(const MacAddress& bssid)
	{
		for(unsigned i = 0; i < this->count(); ++i) {
			if(this->elementAt(i).bssid == bssid) {
				return i;
			}
		}

		return -1;
	}
};

/**
 * @brief For applications to use to manage list of unique beacons
 */
using BeaconInfoList = BeaconOrClientListTemplate<BeaconInfo>;

/**
 * @brief For applications to use to manage list of unique clients
 */
using ClientInfoList = BeaconOrClientListTemplate<ClientInfo>;

using WifiSnifferCallback = Delegate<void(uint8_t* data, uint16_t length)>;
using WifiBeaconCallback = Delegate<void(const BeaconInfo& beacon)>;
using WifiClientCallback = Delegate<void(const ClientInfo& client)>;

class WifiSniffer : public ISystemReadyHandler
{
public:
	/** @brief Initialise the sniffer */
	void begin();

	/** @brief Stop the sniffer */
	void end();

	/** @brief Register notification for beacon (AP) info */
	void onBeacon(WifiBeaconCallback callback)
	{
		beaconCallback = callback;
	}

	/** @brief Register notification for client info */
	void onClient(WifiClientCallback callback)
	{
		clientCallback = callback;
	}

	/** @brief Register notification for all incoming data
	 *  @note Callback invoked for all packet types, including beacon/client
	 */
	void onSniff(WifiSnifferCallback callback)
	{
		snifferCallback = callback;
	}

	/** @brief Set the channel to listen on
	 *  @param channel
	 */
	void setChannel(unsigned channel)
	{
		wifi_set_channel(channel);
	}

	/** @brief Get the current channel being listened on */
	unsigned getChannel()
	{
		return wifi_get_channel();
	}

private:
	/** @brief Perform actual initialisation only when system has been fully initialised */
	void onSystemReady() override;

	/** @brief Parse received Wifi data */
	static void parseData(uint8_t* buf, uint16_t len);

	static WifiSnifferCallback snifferCallback;
	static WifiBeaconCallback beaconCallback;
	static WifiClientCallback clientCallback;
};

/** @} */
