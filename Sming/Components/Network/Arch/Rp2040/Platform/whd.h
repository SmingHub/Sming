/****
 * Sming Framework Project - Open Source framework for high efficiency native ESP8266 development.
 * Created 2015 by Skurydin Alexey
 * http://github.com/SmingHub/Sming
 * All files of the Sming Core are provided under the LGPL v3 license.
 *
 * whd.h - Infineon WiFi Host Driver definitions
 *
 * See https://github.com/infineon/wifi-host-driver
 *
 ****/

#pragma once

#include <WString.h>
#include <MacAddress.h>

#ifndef CYW43_IOCTL_GET_BSSID
#define CYW43_IOCTL_GET_BSSID 0x2E
#endif

struct wlc_ssid_t {
	uint32_t SSID_len;
	uint8_t SSID[32];
};

struct channel_info_t {
	int32_t hw_channel;
	int32_t target_channel;
	int32_t scan_channel;
};

using whd_mac_t = MacAddress::Octets;

struct wl_rx_mgmt_data_t {
	uint16_t version;
	uint16_t channel;
	int32_t rssi;
	uint32_t mactime;
	uint32_t rate;
};

// From linux kernel hostap_80211.h
struct hostap_ieee80211_mgmt_t {
	uint16_t frame_control;
	uint16_t duration;
	whd_mac_t da[6];
	whd_mac_t sa[6];
	whd_mac_t bssid[6];
	uint16_t seq_ctrl;
	union {
		struct {
			uint16_t auth_alg;
			uint16_t auth_transaction;
			uint16_t status_code;
			/* possibly followed by Challenge text */
			uint8_t variable[0];
		} __packed auth;
		struct {
			uint16_t reason_code;
		} __packed deauth;
		struct {
			uint16_t capab_info;
			uint16_t listen_interval;
			/* followed by SSID and Supported rates */
			uint8_t variable[0];
		} __packed assoc_req;
		struct {
			uint16_t capab_info;
			uint16_t status_code;
			uint16_t aid;
			/* followed by Supported rates */
			uint8_t variable[0];
		} __packed assoc_resp, reassoc_resp;
		struct {
			uint16_t capab_info;
			uint16_t listen_interval;
			whd_mac_t current_ap[6];
			/* followed by SSID and Supported rates */
			uint8_t variable[0];
		} __packed reassoc_req;
		struct {
			uint16_t reason_code;
		} __packed disassoc;
		struct {
		} __packed probe_req;
		struct {
			uint8_t timestamp[8];
			uint16_t beacon_int;
			uint16_t capab_info;
			/* followed by some of SSID, Supported rates,
			* FH Params, DS Params, CF Params, IBSS Params, TIM */
			uint8_t variable[0];
		} __packed beacon, probe_resp;
	} u;
} __packed;

String whd_get_ssid(int itf);
MacAddress whd_get_bssid(int itf);
uint8_t whd_get_channel(int itf);
