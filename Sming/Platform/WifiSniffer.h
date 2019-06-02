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

#include "System.h"
#include "WVector.h"

#define ETH_MAC_LEN 6

/**
 * @brief Decoded Wifi beacon (Access Point) information
 */
struct BeaconInfo {
	uint8_t bssid[ETH_MAC_LEN];
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
	uint8_t bssid[ETH_MAC_LEN];
	uint8_t station[ETH_MAC_LEN];
	uint8_t ap[ETH_MAC_LEN];
	uint8_t channel;
	int8_t err;
	int8_t rssi;
	uint16_t seq_n;
};

/**
 * @brief For applications to use to manage list of unique beacons
 */
class BeaconInfoList : public Vector<BeaconInfo>
{
public:
	int indexOf(const uint8_t bssid[])
	{
		for(unsigned i = 0; i < count(); ++i) {
			if(memcmp(elementAt(i).bssid, bssid, ETH_MAC_LEN) == 0) {
				return i;
			}
		}

		return -1;
	}
};

/**
 * @brief For applications to use to manage list of unique clients
 */
class ClientInfoList : public Vector<ClientInfo>
{
public:
	int indexOf(const uint8_t station[])
	{
		for(unsigned i = 0; i < count(); ++i) {
			if(memcmp(elementAt(i).station, station, ETH_MAC_LEN) == 0) {
				return i;
			}
		}

		return -1;
	}
};

typedef std::function<void(uint8_t* data, uint16_t length)> WifiSnifferCallback;
typedef std::function<void(const BeaconInfo& beacon)> WifiBeaconCallback;
typedef std::function<void(const ClientInfo& client)> WifiClientCallback;

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
