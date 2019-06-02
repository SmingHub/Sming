/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * WifiSniffer.cpp
 *
 * Original code by Ray Burnette http://www.hackster.io/rayburne/projects
 *
 * Adapted for use with Sming March 2019 mikee47 <mike@sillyhouse.net>
 *
 ****/

#include "Platform/WifiSniffer.h"
#include "esp_wifi_sniffer.h"

WifiSnifferCallback WifiSniffer::snifferCallback;
WifiBeaconCallback WifiSniffer::beaconCallback;
WifiClientCallback WifiSniffer::clientCallback;

static const uint8_t broadcast1[3] = {0x01, 0x00, 0x5e};
static const uint8_t broadcast2[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static const uint8_t broadcast3[3] = {0x33, 0x33, 0x00};

static void parseClientInfo(ClientInfo& ci, uint8_t* frame, uint16_t framelen, int rssi, unsigned channel)
{
	ci.channel = channel;
	ci.err = 0;
	ci.rssi = rssi;

	uint8_t* bssid;
	uint8_t* station;
	uint8_t* ap;

	uint8_t ds = frame[1] & 3;
	switch(ds) {
	// p[1] - xxxx xx00 => NoDS   p[4]-DST p[10]-SRC p[16]-BSS
	case 0:
		bssid = frame + 16;
		station = frame + 10;
		ap = frame + 4;
		break;
	// p[1] - xxxx xx01 => ToDS   p[4]-BSS p[10]-SRC p[16]-DST
	case 1:
		bssid = frame + 4;
		station = frame + 10;
		ap = frame + 16;
		break;
	// p[1] - xxxx xx10 => FromDS p[4]-DST p[10]-BSS p[16]-SRC
	case 2:
		bssid = frame + 10;
		// hack - don't know why it works like this...
		if(memcmp(frame + 4, broadcast1, 3) || memcmp(frame + 4, broadcast2, 3) || memcmp(frame + 4, broadcast3, 3)) {
			station = frame + 16;
			ap = frame + 4;
		} else {
			station = frame + 4;
			ap = frame + 16;
		}
		break;
	// p[1] - xxxx xx11 => WDS    p[4]-RCV p[10]-TRM p[16]-DST p[26]-SRC
	case 3:
	default:
		bssid = frame + 10;
		station = frame + 4;
		ap = frame + 4;
		break;
	}

	memcpy(ci.station, station, ETH_MAC_LEN);
	memcpy(ci.bssid, bssid, ETH_MAC_LEN);
	memcpy(ci.ap, ap, ETH_MAC_LEN);

	ci.seq_n = (frame[23] * 0xFF) + (frame[22] & 0xF0);
}

static void parseBeaconInfo(BeaconInfo& bi, uint8_t* frame, uint16_t framelen, int rssi)
{
	bi.ssid_len = 0;
	bi.channel = 0;
	bi.err = 0;
	bi.rssi = rssi;
	int pos = 36;

	if(frame[pos] == 0x00) {
		while(pos < framelen) {
			switch(frame[pos]) {
			case 0x00: //SSID
				bi.ssid_len = (int)frame[pos + 1];
				if(bi.ssid_len == 0) {
					memset(bi.ssid, '\0', sizeof(bi.ssid));
					break;
				}
				if(bi.ssid_len < 0) {
					bi.err = -1;
					break;
				}
				if(bi.ssid_len > 32) {
					bi.err = -2;
					break;
				}
				memset(bi.ssid, '\0', sizeof(bi.ssid));
				memcpy(bi.ssid, frame + pos + 2, bi.ssid_len);
				bi.err = 0; // before was error??
				break;
			case 0x03: //Channel
				bi.channel = (int)frame[pos + 2];
				pos = -1;
				break;
			default:
				break;
			}
			if(pos < 0)
				break;
			pos += (int)frame[pos + 1] + 2;
		}
	} else {
		bi.err = -3;
	}

	bi.capa[0] = frame[34];
	bi.capa[1] = frame[35];
	memcpy(bi.bssid, frame + 10, ETH_MAC_LEN);
}

void WifiSniffer::parseData(uint8_t* buf, uint16_t len)
{
	if(snifferCallback) {
		snifferCallback(buf, len);
	}

	if(len == 12) {
		//auto data = reinterpret_cast<RxControl*>(buf);
	} else if(len == 128) {
		if(beaconCallback) {
			auto data = reinterpret_cast<sniffer_buf2*>(buf);
			BeaconInfo beacon;
			parseBeaconInfo(beacon, data->buf, 112, data->rx_ctrl.rssi);
			beaconCallback(beacon);
		}
	} else {
		auto data = reinterpret_cast<struct sniffer_buf*>(buf);
		//Is data or QOS?
		if(data->buf[0] == 0x08 || data->buf[0] == 0x88) {
			if(clientCallback) {
				ClientInfo ci;
				parseClientInfo(ci, data->buf, 36, data->rx_ctrl.rssi, data->rx_ctrl.channel);
				// Check BSSID and station don't match (why ?)
				if(memcmp(ci.bssid, ci.station, ETH_MAC_LEN) != 0) {
					clientCallback(ci);
				}
			}
		}
	}
}

void WifiSniffer::begin()
{
	wifi_set_opmode_current(STATION_MODE); // Promiscuous works only with station mode
	System.onReady(this);
}

void WifiSniffer::onSystemReady()
{
	// Set up promiscuous callback
	wifi_station_disconnect(); // As per Espressif docs.
	wifi_promiscuous_enable(false);
	wifi_set_promiscuous_rx_cb(parseData);
	wifi_promiscuous_enable(true);
}

void WifiSniffer::end()
{
	wifi_promiscuous_enable(false);
}
