/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/anakod/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiSniffer.h
 *
 * Original code by Ray Burnette http://www.hackster.io/rayburne/projects
 *
 * Adapted for use with Sming March 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#ifndef _SMING_CORE_WIFI_SNIFFER_H_
#define _SMING_CORE_WIFI_SNIFFER_H_

#include <functional>
#include <WVector.h>

#define ETH_MAC_LEN 6

/**
 * @brief Decoded Wifi beacon (Access Point) information
 */
struct BeaconInfo {
	uint8_t bssid[ETH_MAC_LEN];
	uint8_t ssid[33];
	int ssid_len;
	int channel;
	int err;
	signed rssi;
	uint8_t capa[2];
};

/**
 * @brief Decoded Wifi client information
 */
struct ClientInfo {
	uint8_t bssid[ETH_MAC_LEN];
	uint8_t station[ETH_MAC_LEN];
	uint8_t ap[ETH_MAC_LEN];
	int channel;
	int err;
	signed rssi;
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

typedef std::function<void(const BeaconInfo& beacon)> NewBeaconCallback;
typedef std::function<void(const ClientInfo& client)> NewClientCallback;

class WifiSniffer
{
public:
	/** @brief Initialise the sniffer
	 *  @param beaconCallback when beacon information decoded
	 *  @param clientCallback when client information decoded
	 */
	void begin(NewBeaconCallback beaconCallback, NewClientCallback clientCallback);

	/** @brief Stop the sniffer */
	void end();

	/** @brief Set the channel to listen on
	 *  @param channel
	 */
	void setChannel(unsigned channel);

private:
	static void promisc_cb(uint8_t* buf, uint16_t len);

	static NewBeaconCallback newBeaconCallback; ///< Registered callback when new beacon found
	static NewClientCallback newClientCallback; ///< Registered callback when new client found
};

#endif /* _SMING_CORE_WIFI_SNIFFER_H_ */
